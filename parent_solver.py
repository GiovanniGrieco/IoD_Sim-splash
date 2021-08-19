#!/usr/bin/python3
import argparse
import json

UNRECOVERABLE_ORPHANS_THRESHOLD = 10

class Graph:
    def __init__(self):
        self.root = {'Object': {
            '__attrs': []
        }}

    def add_node(self, parent, child, attrs):
        inserted = self.__add_node(self.root, parent, child, attrs)
        if not inserted:
            self.root[parent] = {}
            self.root[parent][child] = {
                '__attrs': attrs
            }

    def __add_node(self, root, parent, child, attrs):
        if parent in root:
            if child not in root[parent]:
                root[parent][child] = {
                    '__attrs': attrs
                }
            return True
        else:
            for c in root.keys():
                if c == '__attrs': continue

                ret = self.__add_node(root[c], parent, child, attrs)
                if ret: return True

        return False

    def recover_orphans(self):
        unrecoverable = 0
        # Only Object should remain, ideally
        while len(self.root) > 1 and unrecoverable < UNRECOVERABLE_ORPHANS_THRESHOLD:
            orphans = [k for k in self.root.keys() if k not in ['Object', '__attrs']]

            for o in orphans:
                inserted = self.__append_node(self.root['Object'], o, self.root[o])
                if inserted:
                    del self.root[o]
                else:
                    #print(f'Warning: {o} is unrecoverable')
                    unrecoverable += 1

    def __append_node(self, root, node_name, node):
        if node_name in root:
            root[node_name].update(node)
            return True
        else:
            for c in root.keys():
                if c == '__attrs': continue

                ret = self.__append_node(root[c], node_name, node)
                if ret: return True

        return False

    def downshift_attributes(self):
        for r in self.root.keys():
            self.__downshift_attributes(self.root[r])

    def __downshift_attributes(self, root):
        attrs = root['__attrs'] if '__attrs' in root else None
        for c in [x for x in root.keys() if x != '__attrs']:
            if attrs is not None:
                root[c]['__attrs'].extend(attrs)
            self.__downshift_attributes(root[c])

    def update_on(self, models):
        for r in self.root.keys():
            self.__update_on(self.root[r], r, models)

    def __update_on(self, root, root_name, models):
        # search root_name in models
        for m in models:
            if m['name'] == root_name:
                if '__attrs' in root:
                    m['attributes'] = root['__attrs']
                break
        else:
            for c in [x for x in root.keys() if x != '__attrs']:
                self.__update_on(root[c], c, models)


class App:
    def __init__(self):
        self.__parse_args()

    def run(self):
        g = Graph()
        models = None

        # import
        with open(self.__irs_file, 'r') as f:
            models = json.loads(f.read())

            for m in models:
                g.add_node(m['parent'].replace('ns3::', ''), m['name'], m['attributes'])

        # transform
        g.recover_orphans()
        g.downshift_attributes()
        g.update_on(models)

        for m in models:
            del m['parent']

        # export
        with open(self.__out_file, 'w') as f:
            f.write(json.dumps(models))


    def __parse_args(self):
        P = argparse.ArgumentParser()
        P.add_argument('irs_file', type=str, help='Input IRS JSON file.')
        P.add_argument('irs_output', type=str, help='Output IRS JSON file.')

        A = P.parse_args()
        self.__irs_file = A.irs_file
        self.__out_file = A.irs_output


if __name__ == '__main__':
    App().run()
