// $MinimumShaderProfile: ps_2_0

// (C) 2011 Jan-Willem Krans (janwillem32 <at> hotmail.com) released under GPL v2; see COPYING.txt

// Correct video colorspace BT.601 [SD] to BT.709 [HD] for HD video input
// Use this shader only if BT.709 [HD] encoded video is incorrectly matrixed to full range RGB with the BT.601 [SD] colorspace.

sampler s0;
float2  c0;

float4 main(float2 tex : TEXCOORD0) : COLOR {
	float4 si = tex2D(s0, tex); // original pixel
	if (c0.x < 1120 && c0.y < 630) {
		return si; // this shader does not alter SD video
	}
	float3 s1 = si.rgb;
	s1 = s1.rrr * float3(0.299, -0.1495 / 0.886, 0.5) + s1.ggg * float3(0.587, -0.2935 / 0.886, -0.2935 / 0.701) + s1.bbb * float3(0.114, 0.5, -0.057 / 0.701); // RGB to Y'CbCr, BT.601 [SD] colorspace
	return (s1.rrr + float3(0, -0.1674679 / 0.894, 1.8556) * s1.ggg + float3(1.5748, -0.4185031 / 0.894, 0) * s1.bbb).rgbb; // Y'CbCr to RGB output, BT.709 [HD] colorspace
}
