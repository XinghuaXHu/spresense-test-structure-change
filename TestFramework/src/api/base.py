#! /usr/bin/env python3


class NonCloneable(object):
    """Base class for all non copyable/cloneable objects."""

    def __copy__(self):
        return self

    def __deepcopy__(self, memo):
        return self
