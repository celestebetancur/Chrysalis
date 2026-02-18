SndBuf buf => dac;
adc.chan(0) => blackhole;

me.dir() + "special:dope" => buf.read;

0.5 => buf.rate;
1 => buf.loop;
0 => buf.pos;
4 => dac.gain;

while(true){
    adc.last() => buf.rate;
    second => now;
}