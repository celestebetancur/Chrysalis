// Globals to interact with the knobs in VCV Rack
// Chrysalis_1
// Chrysalis_2
// Chrysalis_3
// Chrysalis_4

global float Chrysalis_1;

// The ChucK code
adc.chan(0) => Gain g => JCRev rev => dac;
g => Gain feedback => DelayL delay => g;

330::ms => delay.max => delay.delay;

.99 => delay.gain;
.8 => feedback.gain;

// Infinite loop at the speed you want to update your variables. samp => now; means at sample rate speed. (Host dependant)
while(true){
    Chrysalis_1 => feedback.gain;
    second => now;
}
