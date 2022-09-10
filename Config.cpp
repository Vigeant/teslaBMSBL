#include "Config.hpp"

template <>
void ParamImpl<uint32_t>::prettyPrint() {
  Serial.printf("%35s = %-15u\n", paramName, value);
}
template <>
void ParamImpl<int32_t>::prettyPrint() {
  Serial.printf("%35s = %-15d\n", paramName, value);
}
template <>
void ParamImpl<float>::prettyPrint() {
  Serial.printf("%35s = %-15.3f\n", paramName, value);
}
