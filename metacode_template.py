from NIENV import *

class %CLASS%(NodeInstance):
    def __init__(self, params):
        super(%CLASS%, self).__init__(params)

    def update_event(self, input_called=-1):
        self.set_output_val(0, %OUTPUT_CONSTRUCT%)

    def get_data(self):
        data = {}
        return data

    def set_data(self, data):
        pass

    def removing(self):
        pass