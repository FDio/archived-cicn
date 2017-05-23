#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
# This module is derived from code from SQLAlchemy
#
# orm/collections.py
# Copyright (C) 2005-2016 the SQLAlchemy authors and contributors
#
# This module is part of SQLAlchemy and is released under
# the MIT License: http://www.opensource.org/licenses/mit-license.php
#

import logging

from vicn.core.exception    import VICNListException
from vicn.core.sa_compat    import py2k
from vicn.core.state        import UUID

log = logging.getLogger(__name__)

def _list_decorators():
    """Tailored instrumentation wrappers for any list-like class."""

    def _tidy(fn):
        fn._sa_instrumented = True
        fn.__doc__ = getattr(list, fn.__name__).__doc__

    def append(fn):
        def append(self, item):
            try:
                item = self._attribute.do_list_add(self._instance, item)
                fn(self, item)
            except VICNListException as e:
                pass
        _tidy(append)
        return append

    def remove(fn):
        def remove(self, value):
            # testlib.pragma exempt:__eq__
            try:
                self._attribute.do_list_remove(self._instance, value)
                fn(self, value)
            except : pass
        _tidy(remove)
        return remove

    def insert(fn):
        def insert(self, index, value):
            try:
                value = self._attribute.do_list_add(self._instance, item)
                fn(self, index, value)
            except : pass
        _tidy(insert)
        return insert

    def __getitem__(fn):
        def __getitem__(self, index):
            item = fn(self, index)
            return self._attribute.handle_getitem(self._instance, item)
        _tidy(__getitem__)
        return __getitem__

    def __setitem__(fn):
        def __setitem__(self, index, value):
            if not isinstance(index, slice):
                existing = self[index]
                if existing is not None:
                    try:
                        self._attribute.do_list_remove(self._instance, existing)
                    except: pass
                    try:
                        value = self._attribute.do_list_add(self._instance, value)
                        fn(self, index, value)
                    except: pass
            else:
                # slice assignment requires __delitem__, insert, __len__
                step = index.step or 1
                start = index.start or 0
                if start < 0:
                    start += len(self)
                if index.stop is not None:
                    stop = index.stop
                else:
                    stop = len(self)
                if stop < 0:
                    stop += len(self)

                if step == 1:
                    for i in range(start, stop, step):
                        if len(self) > start:
                            del self[start]

                    for i, item in enumerate(value):
                        self.insert(i + start, item)
                else:
                    rng = list(range(start, stop, step))
                    if len(value) != len(rng):
                        raise ValueError(
                            "attempt to assign sequence of size %s to "
                            "extended slice of size %s" % (len(value),
                                                           len(rng)))
                    for i, item in zip(rng, value):
                        self.__setitem__(i, item)
        _tidy(__setitem__)
        return __setitem__

    def __delitem__(fn):
        def __delitem__(self, index):
            if not isinstance(index, slice):
                item = self[index]
                try:
                    self._attribute.do_list_remove(self._instance, item)
                    fn(self, index)
                except : pass
            else:
                # slice deletion requires __getslice__ and a slice-groking
                # __getitem__ for stepped deletion
                # note: not breaking this into atomic dels
                has_except = False
                for item in self[index]:
                    try:
                        self._attribute.do_list_remove(self._instance, item)
                    except : has_except = True
                if not has_except:
                    fn(self, index)
        _tidy(__delitem__)
        return __delitem__

    if py2k:
        def __setslice__(fn):
            def __setslice__(self, start, end, values):
                has_except = False
                for value in self[start:end]:
                    try:
                        self._attribute.do_list_remove(self._instance, value)
                    except : has_except = True
                #values = [self._attribute.do_list_add(self._instance, value) for value in values]
                _values = list()
                for value in values:
                    try:
                        _values.append(self._attribute.do_list_add(self._instance, value))
                    except: has_except = True
                if not has_except:
                    fn(self, start, end, _values)
            _tidy(__setslice__)
            return __setslice__

        def __delslice__(fn):
            def __delslice__(self, start, end):
                has_except = False
                for value in self[start:end]:
                    try:
                        self._attribute.do_list_remove(self._instance, value)
                    except : has_except = True
                if not has_except:
                    fn(self, start, end)
            _tidy(__delslice__)
            return __delslice__

    def extend(fn):
        def extend(self, iterable):
            for value in iterable:
                self.append(value)
        _tidy(extend)
        return extend

    def __iadd__(fn):
        def __iadd__(self, iterable):
            # list.__iadd__ takes any iterable and seems to let TypeError
            # raise as-is instead of returning NotImplemented
            for value in iterable:
                self.append(value)
            return self
        _tidy(__iadd__)
        return __iadd__

    def pop(fn):
        def pop(self, index=-1):
            try:
                self._attribute.do_list_remove(self._instance, item)
                item = fn(self, index)
                return item
            except : return None
        _tidy(pop)
        return pop

    def __iter__(fn):
        def __iter__(self):
            for item in fn(self):
                yield self._attribute.handle_getitem(self._instance, item)
        _tidy(__iter__)
        return __iter__

    def __repr__(fn):
        def __repr__(self):
            return '<Collection {} {}>'.format(id(self), list.__repr__(self))
        _tidy(__repr__)
        return __repr__

    __str__ = __repr__
    #def __str__(fn):
    #    def __str__(self):
    #        return str(list(self))
    #    _tidy(__str__)
    #    return __str__

    if not py2k:
        def clear(fn):
            def clear(self, index=-1):
                has_except = False
                for item in self:
                    try:
                        self._attribute.do_list_remove(self._instance, item)
                    except : has_except = True
                if not has_except:
                    fn(self)
            _tidy(clear)
            return clear

    # __imul__ : not wrapping this.  all members of the collection are already
    # present, so no need to fire appends... wrapping it with an explicit
    # decorator is still possible, so events on *= can be had if they're
    # desired.  hard to imagine a use case for __imul__, though.

    l = locals().copy()
    l.pop('_tidy')
    return l

def _instrument_list(cls):
    # inspired by sqlalchemy
    for method, decorator in _list_decorators().items():
        fn = getattr(cls, method, None)
        if fn:
            #if (fn and method not in methods and
            #        not hasattr(fn, '_sa_instrumented')):
            setattr(cls, method, decorator(fn))

class InstrumentedList(list):

    @classmethod
    def from_list(cls, value, instance, attribute):
        lst = list()
        if value:
            for x in value:
                if isinstance(x, UUID):
                    x = instance.from_uuid(x)
                lst.append(x)
        # Having a class method is important for inheritance
        value = cls(lst)
        value._attribute = attribute
        value._instance = instance
        return value

    def __contains__(self, key):
        from vicn.core.resource import Resource
        if isinstance(key, Resource):
            key = key.get_uuid()
        return list.__contains__(self, key)

    def __lshift__(self, item):
        self.append(item)

_instrument_list(InstrumentedList)
