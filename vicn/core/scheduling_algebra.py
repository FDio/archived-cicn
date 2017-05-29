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

def SchedulingAlgebra(cls, concurrent_mixin=object, composition_mixin=object,
        sequential_mixin=object): # allow_none = True

    class BaseElement(cls):
        def __default__(cls, *elements):
            elts = [e for e in elements
                if e is not None and not isinstance(e, Empty)]
            if len(elts) == 0:
                # The first is always Empty
                assert len(elements) != 0
                return elements[0]
            elif len(elts) == 1:
                return elts[0]
            return cls(*elts)

        def __concurrent__(*elements):
            return BaseElement.__default__(Concurrent, *elements)

        def __composition__(*elements):
            return BaseElement.__default__(Composition, *elements)

        def __sequential__(*elements):
            return BaseElement.__default__(Sequential, *elements)

         # Operator: |
        __or__ = __concurrent__

        # Operator: >
        __gt__ = __sequential__

        # Operator: @
        __matmul__ = __composition__

    class Element(BaseElement):
        def __iter__(self):
            yield self

    class Operator(BaseElement):
        def __init__(self, *elements):
            super().__init__()
            self._elements = list(elements)

        def __iter__(self):
            yield self
            for element in self._elements:
                for x in element:
                    yield x

    class Concurrent(Operator, concurrent_mixin):
        # Algebraic rule : ((A // B) // C) ~ (A // B // C)
        def __concurrent__(self, other):
            self._elements.append(other)
            return self

        def __repr__(self):
            return '<Concurrent {}>'.format(self._elements)

    class Composition(Operator, composition_mixin):
        def __repr__(self):
            return '<Composition {}>'.format(self._elements)

    class Sequential(Operator, sequential_mixin):
        def __repr__(self):
            return '<Sequential {}>'.format(self._elements)

    class Empty(Element):
        def __concurrent__(self, other):
            return other

        def __composition__(self, other):
            return other

        def __sequential__(self, other):
            return other

        def __repr__(self):
            return '<Empty>'

    return Element, Empty
