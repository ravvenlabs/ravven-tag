import struct
import mmap

class Register:
    def __init__(self, addr, size = 4, count = 1, default = None, signed = False):
        self._mem_map = None
        self.addr = addr
        self._size = size
        self._count = count
        self._default = default
        self._signed = signed
    
    def assignMemMap(self, mem_map):
        self._mem_map = mem_map

    def initReg(self):
        if self._default is not None:
            self.write(self._default)

    def write(self, value):
        if self._count == 1:
            value = [value]
        for cnt, val in zip(range(self._count), value):
            self._mem_map.seek(Register._getAddr(self.addr, cnt, self._size))
            self._mem_map.write(struct.pack(Register._getType(self._signed, self._size), val))

    def read(self):
        ret = [None] * self._count
        for cnt in range(self._count):
            self._mem_map.seek(Register._getAddr(self.addr, cnt, self._size))
            ret[cnt] = self._mem_map.read(self._size)
        return ret[0] if self._count == 1 else ret

    @staticmethod
    def _getType(signed, size):
        type_char = 'x'
        if size == 1:
            type_char = 'b'
        elif size == 2:
            type_char = 'h'
        elif size == 4:
            type_char = 'i'
        elif size == 8:
            type_char = 'q'
        else:
            raise TypeError("Size should be either 1, 2, 4, or 8")
        
        return type_char if signed else type_char.upper()

    @staticmethod
    def _getAddr(base, cnt, size):
        return base + cnt * size

class RegisterMap:
    def __init__(self, length, mem_file="/dev/mem", **kwargs):
        self.registers = {}
        self._filemap = open(mem_file, "r+b")
        self._mem_map = mmap.mmap(self._filemap.fileno(), length, offset=0x81200000)

    def __del__(self):
        self.close()

    def addRegister(self, regName, reg):
        if self.registers.get(regName) is not None:
            print("Overwriting {} with new addr {}".format(regName, reg.addr))
        reg.assignMemMap(self._mem_map)
        self.registers[regName] = reg

    def readRegister(self, regName):
        return self.registers[regName].read()

    def writeRegister(self, regName, value):
        self.registers[regName].write(value)
    
    def initRegisters(self):
        for reg in self.registers.values():
            reg.initReg()

    def close(self):
        self._mem_map.close()
        self._filemap.close()
