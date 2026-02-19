# Chrysalis

The ChucK (https://chuck.stanford.edu/) Programming Language in VCV Rack (https://vcvrack.com/).


## Build

Clone the repository and its submodules:

```bash
git clone --recursive https://github.com/celestebetancur/Chrysalis.git
```

This plugin uses the [Rack SDK](https://vcvrack.com/downloads/Rack-SDK-2.0.0-lin.zip) to build.

1.  Set the `RACK_DIR` environment variable to the path of your extracted Rack SDK.
2.  Run `make` to compile the plugin.

```bash
export RACK_DIR=<path-to-Rack-SDK>
make install
```

## Usage

Use the global float variables `Chrysalis_1` through `Chrysalis_4` to interact with the knobs on the module panel from your `.ck` scripts.

Example script `test.ck`:

```chuck
// Control feedback gain with the first knob
global float Chrysalis_1;

adc => Gain g => dac;
g => DelayL delay => g;
330::ms => delay.max => delay.delay;

while(true) {
    Chrysalis_1 => delay.gain;
    10::ms => now;
}
```
