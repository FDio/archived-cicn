from netmodel.model.mapper         import ObjectSpecification

class Key(ObjectSpecification):
    def __init__(self, *attributes):
        self._attributes = attributes

    #--------------------------------------------------------------------------
    # Descriptor protocol
    #
    # see. https://docs.python.org/3/howto/descriptor.html
    #--------------------------------------------------------------------------

    def __set_name__(self, owner, name):
        self._name = name
        self._owner = owner

    def __iter__(self):
        for attribute in self._attributes:
            yield attribute
