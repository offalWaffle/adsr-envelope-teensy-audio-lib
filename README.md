# adsr-envelope-teensy-audio-lib
ADSR envelope with variable Log and Exp curves. Audiostream inherited class that works with Teensy Audio Library.

Key features:
- ADSR envelope only - now decoupled from VCA. To be used with AudioEffectMultiply class.
- Attack, Decay and Release curves can be made to be linear, logarithmic or exponential.
- Curves saved in lookup tables when object is instantiated. 
- Use standard log and exponential curves or create custom curves.


