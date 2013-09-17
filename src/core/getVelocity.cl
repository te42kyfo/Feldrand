kernel void getVelocity(global float *vel, int width, int height ) {
    const int globalx = get_global_id(0);
    const int globaly = get_global_id(1);

	if( globalx >= width || globaly >= height) return;

	vel[(globaly*width + globalx)*2    ] = globaly-(height/2);
	vel[(globaly*width + globalx)*2 + 1] = globalx-(width/2);
}

