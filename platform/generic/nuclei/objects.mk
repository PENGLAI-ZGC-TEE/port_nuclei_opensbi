#
# SPDX-License-Identifier: BSD-2-Clause
#

carray-platform_override_modules-$(CONFIG_PLATFORM_NUCLEI_EVALSOC) += nuclei_evalsoc
platform-objs-$(CONFIG_PLATFORM_SIFIVE_FU540) += nuclei/evalsoc.o

carray-platform_override_modules-$(CONFIG_PLATFORM_NUCLEI_DEMOSOC) += nuclei_demosoc
platform-objs-$(CONFIG_PLATFORM_SIFIVE_FU740) += nuclei/demosoc.o

carray-platform_override_modules-$(CONFIG_PLATFORM_NUCLEI_CUSTOMSOC) += nuclei_customsoc
platform-objs-$(CONFIG_PLATFORM_SIFIVE_FU740) += nuclei/customsoc.o
