#ifndef BITOPS_H
#define BITOPS_H

/// __typeof__ preprocessor directive lets compiler detect correct type casting for the mask constant 

/// Set bit in provided variable in place. Bit-wise OR between the current value and the mask
#define SET_BIT(val, bit) (val |= (((__typeof__(val))1) << bit))

/// Clear bit in provided variable in place. Bit-wise AND between the current value and the INVERTED mask
#define CLEAR_BIT(val, bit) (val &= ~(((__typeof__(val))1) << bit))

/// Read value of a single bit. Returns 1 or 0. Bitwise AND between the current value and mask is used as test. 
/// Non-zero will return 1, Zero will return 0
#define READ_BIT(val, bit) ((val & (((__typeof__(val))1) << bit)) ? 1 : 0)

#endif
