#ifndef UTIL_KMS_H
#define UTIL_KMS_H

const char *util_lookup_encoder_type_name(unsigned int type);
const char *util_lookup_connector_status_name(unsigned int type);
const char *util_lookup_connector_type_name(unsigned int type);

int util_open(const char *device, const char *module);

#endif /* UTIL_KMS_H */
