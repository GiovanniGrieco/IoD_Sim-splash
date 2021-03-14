#!/usr/bin/python

import argparse
import os
import json
from typing import List, Optional

def get_current_script_path() -> str:
        return os.path.dirname(__file__)

class NodeGenerator:
    def __init__(self, input_file, output_directory, package_name):
        self.input_file = input_file
        self.output_directory = output_directory
        self.package_name = package_name

        self.package_directory = f'{output_directory}/{self.package_name}'
        os.mkdir(self.package_directory)

    def run(self):
        ir = self._import_ir()
        self._create_rpc_file(ir)

        with open(f'{get_current_script_path()}/metacode_template.py', 'r') as f:
            template_code = f.read()

        os.mkdir(f'{self.package_directory}/nodes')
        for node in ir:
            node_dir = f'{self.package_directory}/nodes/{self.package_name}___{node["name"]}0'
            os.mkdir(node_dir)

            node_outputs = []
            i = 0
            for attr in node['attributes']:
                type = self._sim_type_lookup(attr['type'])
                input_metacode = f'self.input({i})' if type is None else f'{type}(self.input({i}))'

                node_outputs.append(f'if self.input({i}): d["attributes"].append({{"name": "{attr["name"]}", "value": {input_metacode}}})')
                i += 1

            newline_char = '\n        '
            node_metacode = template_code
            node_metacode = node_metacode.replace('%NODE_NAME%', f'"ns3::{node["name"]}"')
            node_metacode = node_metacode.replace('%POPULATE_DICTIONARY%', f'{newline_char.join(node_outputs)}')

            with open(f'{node_dir}/{self.package_name}___{node["name"]}0___METACODE.py', 'w') as f:
                f.write(node_metacode)

    def _import_ir(self) -> List:
        with open(self.input_file, 'r') as f:
            file_contents = f.read()

        deserialized = json.loads(file_contents)
        # filter out models that do not provide attributes
        deserialized = [d for d in deserialized if len(d['attributes']) > 0]

        return deserialized

    def _create_rpc_file(self, ir):
        # bootstrap output Ryven structure
        output = {
            "type": "Ryven nodes package",
            "nodes": []
        }

        for node in ir:
            ryven_node_def = {
                'title': node['name'],
                'description': '',
                'type': '',
                'module name': f'{self.package_name}___{node["name"]}0',
                'class name': node['name'],
                'design style': 'extended',
                'color': '#d50000',
                'has main widget': False,
                'custom input widgets': [],
                'inputs': [],
                'outputs': [{
                    'type': 'data',
                    'label': ''
                }]
            }

            for attr in node['attributes']:
                ryven_node_def['inputs'].append({
                    'type': 'data',
                    'label': attr['name'],
                    'has widget': False
                })

            output['nodes'].append(ryven_node_def)

        with open(f'{self.package_directory}/{self.package_name}.rpc', 'w') as f:
            f.write(json.dumps(output))

    @staticmethod
    def _sim_type_lookup(type) -> Optional[str]:
        if type in ['ns3::DoubleValue', 'ns3::TimeValue']:
            return 'float'
        elif type == 'ns3::BooleanValue':
            return 'bool'
        elif type == 'ns3::UintegerValue':
            return 'int'
        else:
            return None


def parse_args():
    P = argparse.ArgumentParser()

    P.add_argument('input', help='Splash IR File Input', type=str)
    P.add_argument('output_directory', help='Output base directory to generate Airflow package.', type=str)
    P.add_argument('-p', required=True, type=str)

    return P.parse_args()

if __name__ == '__main__':
    A = parse_args()
    N = NodeGenerator(A.input, A.output_directory, A.p)

    N.run()
