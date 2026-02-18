SinOsc sin => ADSR env => dac;

while(true){
    env.keyOn();
    Math.random2(100, 350)::ms => now;
    env.keyOff();
    Math.random2(100, 350)::ms => now;
}