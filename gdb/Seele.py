import sys
sys.path.insert(0, '/usr/share/gdb/python/')
import gdb.printing
import glm_pp

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

def build_pretty_printer():
    pp = gdb.printing.RegexpCollectionPrettyPrinter("Seele")
    pp.add_printer("Array", r'^Seele::Array<.*>$', ArrayPrinter)
    return pp

gdb.printing.register_pretty_printer(gdb.current_objfile(), build_pretty_printer())