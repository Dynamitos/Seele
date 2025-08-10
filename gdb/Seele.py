import sys
sys.path.insert(0, '/usr/share/gdb/python/')
import gdb.printing

class ArrayPrinter:
    def __init__(self, val):
        self.val = val

    def to_string(self):
        return f"Array(size={int(self.val['arraySize'])})"
    
    def children(self):
        data = self.val['_data']
        size = int(self.val['arraySize'])
        allocated = int(self.val['allocated'])
        #yield f"[size]", size
        #yield f"[allocated]", allocated
        for i in range(size):
            elem_val = (data + i).dereference()
            yield f"[{i}]", elem_val 
        
    def display_hint(self):
        return "array"

class VectorPrinter:
    def __init__(self, val):
        self.val = val
        t = val.type.strip_typedefs()
        self.length = int(t.template_argument(0))
        self.value_type = t.template_argument(1)
    
    def to_string(self):
        comps = [float(self.val['x']), float(self.val['y'])]
        if self.length >= 3:
            comps.append(float(self.val['z']))
        if self.length == 4:
            comps.append(float(self.val['w']))
        return "(" + ", ".join(map(str, comps)) + ")"

    def display_hint(self):
        return "string"

    def _children(self):
        comps = {'x': self.val['x'], 'y': self.val['y']}
        if self.length >= 3:
            comps['z'] = self.val['z']
        if self.length == 4:
            comps['w'] = self.val['w']
        for key, value in comps.items():
            yield key, value


def build_pretty_printer():
    pp = gdb.printing.RegexpCollectionPrettyPrinter("Seele")
    pp.add_printer("Array", r'^Seele::Array<.*>$', ArrayPrinter)
    pp.add_printer("Vector", r"^glm::(detail::)?t?vec(\d)?<[^<>]*>$", VectorPrinter)
    return pp


gdb.printing.register_pretty_printer(gdb.current_objfile(), build_pretty_printer())