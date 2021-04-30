import numpy as np

def QX_XtoFloat(value, word_len, frac_len, signed=True):
    # Convert to signed
    if signed:
        value = (int(value) ^ (1 << word_len)) - (1 << word_len)
    # Scale
    return float(value) * (1.0 / (2.0**frac_len))

def FloatToQX_X(value, frac_len, signed=True):
    return int(np.floor(value * (1 << frac_len)))
