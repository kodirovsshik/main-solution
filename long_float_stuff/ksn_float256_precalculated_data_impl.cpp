#include <ksn/ksn.hpp>

_KSN_BEGIN

const char* __precalculated_power_of_ten_data = "\x1\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\xa\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x64\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\xe8\x3\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x10\x27\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\xa0\x86\x1\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x40\x42\xf\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x80\x96\x98\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\xe1\xf5\x5\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\xca\x9a\x3b\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\xe4\xb\x54\x2\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\xe8\x76\x48\x17\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x10\xa5\xd4\xe8\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\xa0\x72\x4e\x18\x9\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x40\x7a\x10\xf3\x5a\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x80\xc6\xa4\x7e\x8d\x3\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\xc1\x6f\xf2\x86\x23\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x8a\x5d\x78\x45\x63\x1\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x64\xa7\xb3\xb6\xe0\xd\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\xe8\x89\x4\x23\xc7\x8a\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x10\x63\x2d\x5e\xc7\x6b\x5\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\xa0\xde\xc5\xad\xc9\x35\x36\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x40\xb2\xba\xc9\xe0\x19\x1e\x2\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x80\xf6\x4a\xe1\xc7\x2\x2d\x15\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\xa1\xed\xcc\xce\x1b\xc2\xd3\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x4a\x48\x1\x14\x16\x95\x45\x8\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\xe4\xd2\xc\xc8\xdc\xd2\xb7\x52\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\xe8\x3c\x80\xd0\x9f\x3c\x2e\x3b\x3\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x10\x61\x2\x25\x3e\x5e\xce\x4f\x20\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\xa0\xca\x17\x72\x6d\xae\xf\x1e\x43\x1\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x40\xea\xed\x74\x46\xd0\x9c\x2c\x9f\xc\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x80\x26\x4b\x91\xc0\x22\x20\xbe\x37\x7e\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x81\xef\xac\x85\x5b\x41\x6d\x2d\xee\x4\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\xa\x5b\xc1\x38\x93\x8d\x44\xc6\x4d\x31\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x64\x8e\x8d\x37\xc0\x87\xad\xbe\x9\xed\x1\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\xe8\x8f\x87\x2b\x82\x4d\xc7\x72\x61\x42\x13\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x10\x9f\x4b\xb3\x15\x7\xc9\x7b\xce\x97\xc0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\xa0\x36\xf4\x0\xd9\x46\xda\xd5\x10\xee\x85\x7\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x40\x22\x8a\x9\x7a\xc4\x86\x5a\xa8\x4c\x3b\x4b\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x80\x56\x65\x5f\xc4\xac\x43\x89\x93\xfe\x50\xf0\x2\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x61\xf5\xb9\xab\xbf\xa4\x5c\xc3\xf1\x29\x63\x1d\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\xca\x95\x43\xb5\x7c\x6f\x9e\xa1\x71\xa3\xdf\x25\x1\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\xe4\xd9\xa3\x14\xdf\x5a\x30\x50\x70\x62\xbc\x7a\xb\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\xe8\x82\x66\xce\xb6\x8c\xe3\x21\x63\xd8\x5b\xcb\x72\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x10\x1d\x1\x10\x24\x7f\xe3\x52\xdf\x73\x96\xf1\x7b\x4\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\xa0\x22\xb\xa0\x68\xf7\xe2\x3c\xb9\x86\xe0\x6f\xd7\x2c\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x40\x5a\x6f\x40\x16\xaa\xdd\x60\x3c\x43\xc5\x5e\x6a\xc0\x1\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x80\x86\x59\x84\xde\xa4\xa8\xc8\x5b\xa0\xb4\xb3\x27\x84\x11\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x41\x7f\x2b\xb1\x70\x96\xd6\x95\x43\xe\x5\x8d\x29\xaf\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x8a\xf8\xb2\xeb\x66\xe0\x61\xda\xa3\x8e\x32\x82\x9f\xd7\x6\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x64\xb5\xfd\x34\x5\xc4\xd2\x87\x66\x92\xf9\x15\x3b\x6c\x44\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\xe8\x15\xe9\x11\x34\xa8\x3b\x4e\x1\xb8\xbf\xdb\x4e\x3a\xac\x2\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x10\xdb\x1a\xb3\x8\x92\x54\xe\xd\x30\x7d\x95\x14\x47\xba\x1a\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\xa0\x8e\xc\xff\x56\xb4\x4d\x8f\x82\xe0\xe3\xd6\xcd\xc6\x46\xb\x1\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x40\x92\x7d\xf6\x65\xb\x9\x99\x19\xc5\xe6\x64\xa\xc4\xc3\x70\xa\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x80\xb6\xe7\xa0\xfb\x71\x5a\xfa\xff\xb2\x3\xf1\x67\xa8\xa5\x67\x68\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x21\xd\x49\xd4\x73\x88\xc7\xff\xfd\x24\x6a\xf\x94\x78\xc\x14\x4\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x4a\x83\xda\x4a\x86\x54\xcb\xfd\xeb\x71\x25\x9a\xc8\xb5\x7c\xc8\x28\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\xe4\x20\x89\xec\x3e\x4d\xf1\xe9\x37\x73\x76\x5\xd6\x19\xdf\xd4\x97\x1\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\xe8\x48\x5b\x3d\x75\x4\x6d\x23\x2f\x80\xa0\x36\x5c\x2\xb7\x50\xee\xf\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x10\xd9\x90\x65\x94\x2c\x42\x62\xd7\x1\x45\x22\x9a\x17\x26\x27\x4f\x9f\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\xa0\x7a\xa8\xf7\xcb\xbd\x95\xd6\x69\x12\xb2\x56\x5\xec\x7c\x87\x17\x39\x6\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x40\xca\x94\xac\xf7\x69\xd9\x61\x22\xb8\xf4\x62\x35\x38\xe1\x4a\xeb\x3a\x3e\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x80\xe6\xcf\xbd\xac\x23\x7e\xd2\x57\x31\x8f\xdd\x15\x32\xcc\xec\x30\x4d\x6e\x2\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x1\x1f\x6a\xbf\x64\xed\x38\x6e\xed\x97\xa7\xda\xf4\xf9\x3f\xe9\x3\x4f\x18\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\xa\x36\x25\x7a\xef\x45\x39\x4e\x46\xef\x8b\x8a\x90\xc3\x7f\x1c\x27\x16\xf3\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x64\x1c\x74\xc5\x5a\xbb\x3c\xe\xbf\x58\x77\x69\xa5\xa3\xfd\x1c\x87\xdd\x7e\x9\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\xe8\x1b\x89\xb6\x8b\x51\x5f\x8e\x76\x77\xa9\x1e\x76\x64\xe8\x21\x47\xa7\xf4\x5e\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x10\x17\x5b\x21\x75\x2f\xb9\x8f\xa1\xaa\x9e\x32\x9d\xec\x13\x53\xc7\x88\x8e\xb5\x3\x0\x0\x0\x0\x0\x0\x0\x0\x0\xa0\xe6\x8e\x4d\x93\xda\x3b\x9d\x4f\xaa\x32\xfa\x23\x3e\xc7\x3e\xc9\x57\x91\x17\x25\x0\x0\x0\x0\x0\x0\x0\x0\x0\x40\x2\x95\x7\xc1\x89\x56\x24\x1c\xa7\xfa\xc5\x67\x6d\xc8\x73\xdc\x6d\xad\xeb\x72\x1\x0\x0\x0\x0\x0\x0\x0\x0\x80\x16\xd2\x4b\x8a\x61\x61\x6b\x19\x87\xca\xbb\xd\x46\xd4\x85\x9c\x4a\xc6\x34\x7d\xe\x0\x0\x0\x0\x0\x0\x0\x0\x0\xe1\x34\xf6\x66\xcf\xcd\x31\xfe\x46\xe9\x55\x89\xbc\x4a\x3a\x1d\xea\xbe\xf\xe4\x90";

_KSN_END
