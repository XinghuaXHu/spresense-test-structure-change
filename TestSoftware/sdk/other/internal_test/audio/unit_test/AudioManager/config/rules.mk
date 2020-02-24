#====================================================================#
#
#       File Name: rules.mk
#
#       Description: CppUTest Sample local rules.
#
#       Notes: (C) Copyright 2015 Sony Corporation
#
#       Author: Tetsuya Sakai
#
#====================================================================#
#ROOT_RULES		= $(ROOT_DIR)/Src/Config/rules.mk
#include $(ROOT_RULES)

%.o:%.cpp $(MAKE_CONFIG) $(LOCAL_DEPENDENCY)
	$(CXX) -c $(CXXFLAGS) -o $@ $<

