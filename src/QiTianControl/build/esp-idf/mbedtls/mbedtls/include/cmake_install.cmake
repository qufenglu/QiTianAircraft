# Install script for directory: G:/esp-idf-v4.0.4/components/mbedtls/mbedtls/include

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "C:/Program Files (x86)/QiTianControl")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "TRUE")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/mbedtls" TYPE FILE PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ FILES
    "G:/esp-idf-v4.0.4/components/mbedtls/mbedtls/include/mbedtls/aes.h"
    "G:/esp-idf-v4.0.4/components/mbedtls/mbedtls/include/mbedtls/aesni.h"
    "G:/esp-idf-v4.0.4/components/mbedtls/mbedtls/include/mbedtls/arc4.h"
    "G:/esp-idf-v4.0.4/components/mbedtls/mbedtls/include/mbedtls/aria.h"
    "G:/esp-idf-v4.0.4/components/mbedtls/mbedtls/include/mbedtls/asn1.h"
    "G:/esp-idf-v4.0.4/components/mbedtls/mbedtls/include/mbedtls/asn1write.h"
    "G:/esp-idf-v4.0.4/components/mbedtls/mbedtls/include/mbedtls/base64.h"
    "G:/esp-idf-v4.0.4/components/mbedtls/mbedtls/include/mbedtls/bignum.h"
    "G:/esp-idf-v4.0.4/components/mbedtls/mbedtls/include/mbedtls/blowfish.h"
    "G:/esp-idf-v4.0.4/components/mbedtls/mbedtls/include/mbedtls/bn_mul.h"
    "G:/esp-idf-v4.0.4/components/mbedtls/mbedtls/include/mbedtls/camellia.h"
    "G:/esp-idf-v4.0.4/components/mbedtls/mbedtls/include/mbedtls/ccm.h"
    "G:/esp-idf-v4.0.4/components/mbedtls/mbedtls/include/mbedtls/certs.h"
    "G:/esp-idf-v4.0.4/components/mbedtls/mbedtls/include/mbedtls/chacha20.h"
    "G:/esp-idf-v4.0.4/components/mbedtls/mbedtls/include/mbedtls/chachapoly.h"
    "G:/esp-idf-v4.0.4/components/mbedtls/mbedtls/include/mbedtls/check_config.h"
    "G:/esp-idf-v4.0.4/components/mbedtls/mbedtls/include/mbedtls/cipher.h"
    "G:/esp-idf-v4.0.4/components/mbedtls/mbedtls/include/mbedtls/cipher_internal.h"
    "G:/esp-idf-v4.0.4/components/mbedtls/mbedtls/include/mbedtls/cmac.h"
    "G:/esp-idf-v4.0.4/components/mbedtls/mbedtls/include/mbedtls/compat-1.3.h"
    "G:/esp-idf-v4.0.4/components/mbedtls/mbedtls/include/mbedtls/config.h"
    "G:/esp-idf-v4.0.4/components/mbedtls/mbedtls/include/mbedtls/ctr_drbg.h"
    "G:/esp-idf-v4.0.4/components/mbedtls/mbedtls/include/mbedtls/debug.h"
    "G:/esp-idf-v4.0.4/components/mbedtls/mbedtls/include/mbedtls/des.h"
    "G:/esp-idf-v4.0.4/components/mbedtls/mbedtls/include/mbedtls/dhm.h"
    "G:/esp-idf-v4.0.4/components/mbedtls/mbedtls/include/mbedtls/ecdh.h"
    "G:/esp-idf-v4.0.4/components/mbedtls/mbedtls/include/mbedtls/ecdsa.h"
    "G:/esp-idf-v4.0.4/components/mbedtls/mbedtls/include/mbedtls/ecjpake.h"
    "G:/esp-idf-v4.0.4/components/mbedtls/mbedtls/include/mbedtls/ecp.h"
    "G:/esp-idf-v4.0.4/components/mbedtls/mbedtls/include/mbedtls/ecp_internal.h"
    "G:/esp-idf-v4.0.4/components/mbedtls/mbedtls/include/mbedtls/entropy.h"
    "G:/esp-idf-v4.0.4/components/mbedtls/mbedtls/include/mbedtls/entropy_poll.h"
    "G:/esp-idf-v4.0.4/components/mbedtls/mbedtls/include/mbedtls/error.h"
    "G:/esp-idf-v4.0.4/components/mbedtls/mbedtls/include/mbedtls/gcm.h"
    "G:/esp-idf-v4.0.4/components/mbedtls/mbedtls/include/mbedtls/havege.h"
    "G:/esp-idf-v4.0.4/components/mbedtls/mbedtls/include/mbedtls/hkdf.h"
    "G:/esp-idf-v4.0.4/components/mbedtls/mbedtls/include/mbedtls/hmac_drbg.h"
    "G:/esp-idf-v4.0.4/components/mbedtls/mbedtls/include/mbedtls/md.h"
    "G:/esp-idf-v4.0.4/components/mbedtls/mbedtls/include/mbedtls/md2.h"
    "G:/esp-idf-v4.0.4/components/mbedtls/mbedtls/include/mbedtls/md4.h"
    "G:/esp-idf-v4.0.4/components/mbedtls/mbedtls/include/mbedtls/md5.h"
    "G:/esp-idf-v4.0.4/components/mbedtls/mbedtls/include/mbedtls/md_internal.h"
    "G:/esp-idf-v4.0.4/components/mbedtls/mbedtls/include/mbedtls/memory_buffer_alloc.h"
    "G:/esp-idf-v4.0.4/components/mbedtls/mbedtls/include/mbedtls/net.h"
    "G:/esp-idf-v4.0.4/components/mbedtls/mbedtls/include/mbedtls/net_sockets.h"
    "G:/esp-idf-v4.0.4/components/mbedtls/mbedtls/include/mbedtls/nist_kw.h"
    "G:/esp-idf-v4.0.4/components/mbedtls/mbedtls/include/mbedtls/oid.h"
    "G:/esp-idf-v4.0.4/components/mbedtls/mbedtls/include/mbedtls/padlock.h"
    "G:/esp-idf-v4.0.4/components/mbedtls/mbedtls/include/mbedtls/pem.h"
    "G:/esp-idf-v4.0.4/components/mbedtls/mbedtls/include/mbedtls/pk.h"
    "G:/esp-idf-v4.0.4/components/mbedtls/mbedtls/include/mbedtls/pk_internal.h"
    "G:/esp-idf-v4.0.4/components/mbedtls/mbedtls/include/mbedtls/pkcs11.h"
    "G:/esp-idf-v4.0.4/components/mbedtls/mbedtls/include/mbedtls/pkcs12.h"
    "G:/esp-idf-v4.0.4/components/mbedtls/mbedtls/include/mbedtls/pkcs5.h"
    "G:/esp-idf-v4.0.4/components/mbedtls/mbedtls/include/mbedtls/platform.h"
    "G:/esp-idf-v4.0.4/components/mbedtls/mbedtls/include/mbedtls/platform_time.h"
    "G:/esp-idf-v4.0.4/components/mbedtls/mbedtls/include/mbedtls/platform_util.h"
    "G:/esp-idf-v4.0.4/components/mbedtls/mbedtls/include/mbedtls/poly1305.h"
    "G:/esp-idf-v4.0.4/components/mbedtls/mbedtls/include/mbedtls/ripemd160.h"
    "G:/esp-idf-v4.0.4/components/mbedtls/mbedtls/include/mbedtls/rsa.h"
    "G:/esp-idf-v4.0.4/components/mbedtls/mbedtls/include/mbedtls/rsa_internal.h"
    "G:/esp-idf-v4.0.4/components/mbedtls/mbedtls/include/mbedtls/sha1.h"
    "G:/esp-idf-v4.0.4/components/mbedtls/mbedtls/include/mbedtls/sha256.h"
    "G:/esp-idf-v4.0.4/components/mbedtls/mbedtls/include/mbedtls/sha512.h"
    "G:/esp-idf-v4.0.4/components/mbedtls/mbedtls/include/mbedtls/ssl.h"
    "G:/esp-idf-v4.0.4/components/mbedtls/mbedtls/include/mbedtls/ssl_cache.h"
    "G:/esp-idf-v4.0.4/components/mbedtls/mbedtls/include/mbedtls/ssl_ciphersuites.h"
    "G:/esp-idf-v4.0.4/components/mbedtls/mbedtls/include/mbedtls/ssl_cookie.h"
    "G:/esp-idf-v4.0.4/components/mbedtls/mbedtls/include/mbedtls/ssl_internal.h"
    "G:/esp-idf-v4.0.4/components/mbedtls/mbedtls/include/mbedtls/ssl_ticket.h"
    "G:/esp-idf-v4.0.4/components/mbedtls/mbedtls/include/mbedtls/threading.h"
    "G:/esp-idf-v4.0.4/components/mbedtls/mbedtls/include/mbedtls/timing.h"
    "G:/esp-idf-v4.0.4/components/mbedtls/mbedtls/include/mbedtls/version.h"
    "G:/esp-idf-v4.0.4/components/mbedtls/mbedtls/include/mbedtls/x509.h"
    "G:/esp-idf-v4.0.4/components/mbedtls/mbedtls/include/mbedtls/x509_crl.h"
    "G:/esp-idf-v4.0.4/components/mbedtls/mbedtls/include/mbedtls/x509_crt.h"
    "G:/esp-idf-v4.0.4/components/mbedtls/mbedtls/include/mbedtls/x509_csr.h"
    "G:/esp-idf-v4.0.4/components/mbedtls/mbedtls/include/mbedtls/xtea.h"
    )
endif()

