#pragma once

#include <cstdlib>
#include <string>

#include <base.hpp>

#include "../policy-rs.hpp"

// sepolicy paths
#define PLAT_POLICY_DIR     "/system/etc/selinux/"
#define VEND_POLICY_DIR     "/vendor/etc/selinux/"
#define PROD_POLICY_DIR     "/product/etc/selinux/"
#define ODM_POLICY_DIR      "/odm/etc/selinux/"
#define SYSEXT_POLICY_DIR   "/system_ext/etc/selinux/"
#define SPLIT_PLAT_CIL      PLAT_POLICY_DIR "plat_sepolicy.cil"

// selinuxfs paths
#define SELINUX_MNT         "/sys/fs/selinux"
#define SELINUX_VERSION     SELINUX_MNT "/policyvers"
