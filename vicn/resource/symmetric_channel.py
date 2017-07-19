from netmodel.model.key         import Key
from vicn.core.attribute        import Attribute
from vicn.core.task             import inherit_parent
from vicn.resource.interface    import Interface
from vicn.resource.channel      import Channel

class SymmetricChannel(Channel):
    src = Attribute(Interface, mandatory = True)
    dst = Attribute(Interface, mandatory = True)

    __key__ = Key(src, dst)

    @inherit_parent
    def __create__(self):
        self.interfaces << self.src
        self.interfaces << self.dst
