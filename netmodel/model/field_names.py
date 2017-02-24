#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
# Copyright (c) 2017 Cisco and/or its affiliates.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at:
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

FIELD_SEPARATOR = '.'
DEFAULT_IS_STAR = False

class FieldNames(list):
    """
    A FieldNames instance gather a set of field_names or represents *.
    THIS IS NOT a set(Field).

    The distinction between parent and children fields is based on the
    FieldNames.FIELD_SEPARATOR character.
    """

    #--------------------------------------------------------------------------
    # Constructor
    #--------------------------------------------------------------------------

    def __init__(self, *args, **kwargs):
        """
        Constructor.
        """
        star = kwargs.pop('star', DEFAULT_IS_STAR)
        list.__init__(self, *args, **kwargs)
        size = len(self)
        if star and size != 0:
            raise ValueError("Inconsistent parameter (star = %s size = %s)" % \
                    (star, size))
        # self._star == False and len(self) == 0 occurs when we create
        # FieldNames() (to use later |=) and must behaves as FieldNames(star =
        # False)
        self._star = star

    def __repr__(self):
        """
        Returns:
            The %r representation of this FieldNames instance.
        """
        if self.is_star():
            return "<FieldNames *>"
        else:
            return "<FieldNames %r>" % [x for x in self]

    def __hash__(self):
        return hash((self._star,) + tuple(self))

    def is_star(self):
        """
        Returns:
            True iif this FieldNames instance correspond to "*".
            Example : SELECT * FROM foo
        """
        try:
            return self._star
        except: 
            # This is due to a bug in early versions of Python 2.7 which are
            # present on PlanetLab. During copy.deepcopy(), the object is
            # reconstructed using append before the state (self.__dict__ is
            # reconstructed). Hence the object has no _star when append is
            # called and this raises a crazy Exception:
            # I could not reproduce in a smaller example
            # http://pastie.org/private/5nf15jg0qcvd05pbmnrp8g
            return False

    def set_star(self):
        """
        Update this FieldNames instance to make it corresponds to "*"
        """
        self._star = True
        self.clear()

    def unset_star(self, field_names):
        """
        Update this FieldNames instance to make it corresponds to a set of
        FieldNames

        Args:
            field_names: A FieldNames instance or a set of String instances
                (field names)
        """
        assert len(field_names) > 0
        self._star = False
        if field_names:
            self |= field_names

    def is_empty(self):
        """
        Returns:
            True iif FieldNames instance designates contains least one field
                name.
        """
        return not self.is_star() and len(self) == 0

    def copy(self):
        """
        Returns:
            A copy of this FieldNames instance.
        """
        return FieldNames(self[:])

    #--------------------------------------------------------------------------
    # Iterators
    #--------------------------------------------------------------------------

    def iter_field_subfield(self):
        for f in self:
            field, _, subfield = f.partition(FIELD_SEPARATOR)
            yield (field, subfield)

    #--------------------------------------------------------------------------
    # Overloaded set internal functions
    #--------------------------------------------------------------------------

    def __or__(self, fields):
        """
        Compute the union of two FieldNames instances.
        Args:
            fields: a set of String (corresponding to field names) or a
                FieldNames instance.
        Returns:
            The union of the both FieldNames instance.
        """
        if self.is_star() or fields.is_star():
            return FieldNames(star = True)
        else:
            l = self[:]
            l.extend([x for x in fields if x not in l])
            return FieldNames(l)

    def __ior__(self, fields):
        """
        Compute the union of two FieldNames instances.
        Args:
            fields: a set of Field instances or a FieldNames instance.
        Returns:
            The updated FieldNames instance.
        """
        if fields.is_star():
            self.set_star()
            return self
        else:
            self.extend([x for x in fields if x not in self])
            return self

    def __and__(self, fields):
        """
        Compute the intersection of two FieldNames instances.
        Args:
            fields: a set of Field instances or a FieldNames instance.
        Returns:
            The intersection of the both FieldNames instances.
        """
        if self.is_star():
            return fields.copy()
        elif isinstance(fields, FieldNames) and fields.is_star():
            return self.copy()
        else:
            return FieldNames([x for x in self if x in fields])

    def __iand__(self, fields):
        """
        Compute the intersection of two FieldNames instances.
        Args:
            fields: a set of Field instances or a FieldNames instance.
        Returns:
            The updated FieldNames instance.
        """
        if self.is_star():
            self.unset_star(fields)
        elif fields.is_star():
            pass
        else:
            self[:] = [x for x in self if x in fields]
        return self

    def __nonzero__(self):
        return self.is_star() or bool(list(self))

    # Python>=3
    __bool__ = __nonzero__

    __add__ = __or__

    def __sub__(self, fields):
        if fields.is_star():
            return FieldNames(star = False)
        else:
            if self.is_star():
                # * - x,y,z = ???
                return FieldNames(star = True)
            else:
                return FieldNames([x for x in self if x not in fields])

    def __isub__(self, fields):
        raise NotImplemented

    def __iadd__(self, fields):
        raise NotImplemented

    #--------------------------------------------------------------------------
    # Overloaded set comparison functions
    #--------------------------------------------------------------------------

    def __eq__(self, other):
        """
        Test whether this FieldNames instance corresponds to another one.
        Args:
            other: The FieldNames instance compared to self.
        Returns:
            True if the both FieldNames instance matches.
        """
        return self.is_star() and other.is_star() or set(self) == set(other) 

    def __le__(self, other):
        """
        Test whether this FieldNames instance in included in
        (or equal to) another one.
        Args:
            other: The FieldNames instance compared to self or
        Returns:
            True if the both FieldNames instance matches.
        """
        assert isinstance(other, FieldNames),\
            "Invalid other = %s (%s)" % (other, type(other))

        return (self.is_star() and other.is_star())\
            or (not self.is_star() and other.is_star())\
            or (set(self) <= set(other)) # list.__le__(self, other)

    # Defined with respect of previous functions

    def __ne__(self, other):
        """
        Test whether this FieldNames instance differs to another one.
        Args:
            other: The FieldNames instance compared to self.
        Returns:
            True if the both FieldNames instance differs.
        """
        return not self == other

    def __lt__(self, other):
        """
        Test whether this FieldNames instance in strictly included in
        another one.
        Args:
            other: The FieldNames instance compared to self.
        Returns:
            True if self is strictly included in other.
        """
        return self <= other and self != other

    def __ge__(self, other):
        return other.__le__(self)

    def __gt__(self, other):
        return other.__lt__(self)

    #--------------------------------------------------------------------------
    # Overloaded set functions
    #--------------------------------------------------------------------------

    def add(self, field_name):
        # DEPRECATED
        assert isinstance(field_name, str)
        self.append(field_name)

    def set(self, field_names):
        assert isinstance(field_names, FieldNames)
        if field_names.is_star():
            self.set_star()
            return
        assert len(field_names) > 0
        self._star = False
        self.clear()
        self |= field_names

    def append(self, field_name):
        if not isinstance(field_name, str):
            raise TypeError("Invalid field_name %s (string expected, got %s)" \
                    % (field_name, type(field_name)))

        if not self.is_star():
            list.append(self, field_name)

    def clear(self):
        self._star = True
        del self[:]

    def rename(self, aliases):
        """
        Rename all the field names involved in self according to a dict.
        Args:
            aliases: A {String : String} mapping the old field name and
                the new field name. 
        Returns:
            The updated FieldNames instance.
        """
        s = self.copy()
        for element in s:
            if element in aliases:
                s.remove(element)
                s.add(aliases[element])
        self.clear()
        self |= s
        return self

    @staticmethod
    def join(field, subfield):
        return "%s%s%s" % (field, FIELD_SEPARATOR, subfield)

    @staticmethod
    def after_path(field, path, allow_shortcuts = True):
        """
        Returns the part of the field after path

        Args:
            path (list):
            allow_shortcuts (bool): Default to True.
        """
        if not path:
            return (field, None)
        last = None
        field_parts = field.split(FIELD_SEPARATOR)
        for path_element in path[1:]:
            if path_element == field_parts[0]:
                field_parts.pop(0)
                last = None
            else:
                last = path_element
        return (FIELD_SEPARATOR.join(field_parts), last)

    def split_subfields(self, include_parent = True, current_path = None, 
            allow_shortcuts = True):
        """
        Args:
            include_parent (bool): is the parent field included in the list of
                returned FieldNames (1st part of the tuple).
            current_path (list): the path of fields that will be skipped at the
                beginning
            path_shortcuts (bool): do we allow shortcuts in the path

        Returns: A tuple made of 4 operands:
            fields:
            map_method_subfields:
            map_original_field:
            rename:

        Example path = ROOT.A.B
        split_subfields(A.B.C.D, A.B.C.D', current_path=[ROOT,A,B]) =>
            (FieldNames(), { C: [D, D'] })
        split_subfields(A.E.B.C.D, A.E.B.C.D', current_path=[ROOT,A,B]) => 
            (FieldNames(), { C: [D, D'] })
        """
        field_names = FieldNames()
        map_method_subfields = dict()
        map_original_field   = dict()
        rename = dict()

        for original_field in self:
            # The current_path can be seen as a set of fields that have to be
            # passed through before we can consider a field
            field, last = FieldNames.after_path(original_field, current_path, 
                    allow_shortcuts)

            field_name, _, subfield = field.partition(FIELD_SEPARATOR)

            if not subfield:
                field_names.add(field_name)
            else:
                if include_parent:
                    field_names.add(field_name)
                if not field_name in map_method_subfields:
                    map_method_subfields[field_name] = FieldNames()
                map_method_subfields[field_name].add(subfield)

            map_original_field[field_name] = original_field
            rename[field_name] = last

        return (field_names, map_method_subfields, map_original_field, rename)
