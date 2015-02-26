AUTOMAKE_OPTIONS=foreign subdir-objects
noinst_LIBRARIES=libencrypt.a
libencrypt_a_SOURCES=./src/encrypt_interface.c \
		     ./src/et199_encrypt.c \
		     ./src/fake.c 

#libencrypt_a_LIBADD=
libencrypt_a_CFLAGS=-Wall -g -fno-inline -fstack-protector -fPIC
#libencrypt_a_CXXFLAGS=

