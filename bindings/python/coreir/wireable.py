import ctypes as ct
from coreir.base import CoreIRType
from coreir.lib import libcoreir_c
from coreir.type import Type, COREArg_p, Arg
import coreir.module

class COREWireable(ct.Structure):
    pass

COREWireable_p = ct.POINTER(COREWireable)


class Wireable(CoreIRType):
    @property
    def connected_wireables(self):
        size = ct.c_int()
        result = libcoreir_c.COREWireableGetConnectedWireables(self.ptr, ct.byref(size))
        return [Wireable(result[i],self.context) for i in range(size.value)]

    @property
    def selectpath(self):
        size = ct.c_int()
        result = libcoreir_c.COREWireableGetSelectPath(self.ptr, ct.byref(size))
        return [result[i].decode() for i in range(size.value)]

    def select(self, field):
        return Select(libcoreir_c.COREWireableSelect(self.ptr, str.encode(field)),self.context)

    @property
    def module_def(self):
        return coreir.module.ModuleDef(libcoreir_c.COREWireableGetModuleDef(self.ptr),self.context)

    @property
    def module(self):
        return self.module_def.module

    @property
    def type(self):
        return Type(libcoreir_c.COREWireableGetType(self.ptr), self.context)


class Select(Wireable):
    pass
    # @property
    # def parent(self):
    #     return Wireable(libcoreir_c.CORESelectGetParent(self.ptr))


class InstanceConfigLazyDict:
    """
    Lazy object that implements the ``instance.config[key]`` interface. Instead
    of building the dictionary explicitly for every call to config, we wait for
    the indexing function to be called and use the
    ``libcoreir_c.COREGetConfigValue`` API
    """
    def __init__(self, parent_instance):
        self.parent_instance = parent_instance

    def __getitem__(self, key):
        return Arg(libcoreir_c.COREGetConfigValue(self.parent_instance.ptr,
                                                  str.encode(key)),
                   self.parent_instance.context)

    def __setitem__(self, key, value):
        raise NotImplementedError()


class Instance(Wireable):
    def __init__(self, ptr, context):
        super().__init__(ptr, context)
        self.config = InstanceConfigLazyDict(self)

    @property
    def module_name(self):
        name = libcoreir_c.COREGetInstRefName(self.ptr)
        return name.decode()

    @property
    def generator_args(self):
        num_args = ct.c_int()
        names = ct.POINTER(ct.c_char_p)()
        args = ct.POINTER(COREArg_p)()
        libcoreir_c.COREInstanceGetGenArgs(self.ptr, ct.byref(names), ct.byref(args), ct.byref(num_args))
        ret = {}
        for i in range(num_args.value):
            ret[names[i].decode()] = Arg(args[i], self.context)
        return ret



class Interface(Wireable):
    pass


class Connection(CoreIRType):
    @property
    def first(self):
        return Wireable(libcoreir_c.COREConnectionGetFirst(self.ptr), self.context)

    @property
    def second(self):
        return Wireable(libcoreir_c.COREConnectionGetSecond(self.ptr), self.context)
