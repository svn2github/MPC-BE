sampler s0 : register(s0);
sampler s1 : register(s1);
sampler s2 : register(s2);
sampler s3 : register(s3);
sampler s4 : register(s4);

float4 dxdy05 : register(c0);
float2 dxdy :   register(c1);
float2 dx :     register(c2);
float2 dy :     register(c3);

#define A _The_Value_Of_A_Is_Set_Here_

// none of the resizers here can be used for 1:1 mapping!
// tex * size won't be 0, 1, 2, 3, ... as you might expect, but something like 0, 0.999, 2.001, 2.999, ...
// this means when the fractional part becomes 0.999 we will be interpolating with the wrong value!!!

struct PS_INPUT {
	float2 t0 : TEXCOORD0;
	float2 t1 : TEXCOORD1;
	float2 t2 : TEXCOORD2;
	float2 t3 : TEXCOORD3;
	float2 t4 : TEXCOORD4;
};

float4 main_bilinear(PS_INPUT input) : COLOR {
	float2 PixelPos = input.t0;
	float2 dd = frac(PixelPos);
	float2 ExactPixel = PixelPos - dd;
	float2 samplePos = ExactPixel * dxdy + dxdy05;

	float4 c = lerp(
		lerp(tex2D(s0, samplePos), tex2D(s0, samplePos + dx), dd.x),
		lerp(tex2D(s0, samplePos + dy), tex2D(s0, samplePos + dxdy), dd.x),
		dd.y);

	return c;
}

static float4x4 tco = {
	0, A, -2 * A, A,
	1, 0, -A - 3, A + 2,
	0, -A, 2 * A + 3, -A - 2,
	0, 0, A, -A
};

float4 taps(float t)
{
	return mul(tco, float4(1, t, t * t, t * t * t));
}

float4 SampleX(float4 tx, float2 t0)
{
	return
		mul(tx,
			float4x4(
				tex2D(s0, t0 - dx),
				tex2D(s0, t0),
				tex2D(s0, t0 + dx),
				tex2D(s0, t0 + dx + dx)
				)
			);
}

float4 SampleY(float4 tx, float4 ty, float2 t0)
{
	return
		mul(ty,
			float4x4(
				SampleX(tx, t0 - dy),
				SampleX(tx, t0),
				SampleX(tx, t0 + dy),
				SampleX(tx, t0 + dy + dy)
				)
			);
}

float4 main_bicubic(PS_INPUT input) : COLOR {
	float2 PixelPos = input.t0;
	float2 dd = frac(PixelPos);
	float2 ExactPixel = PixelPos - dd;
	float2 samplePos = ExactPixel * dxdy + dxdy05;
	return SampleY(taps(dd.x), taps(dd.y), samplePos);
}

float4 Sample(float4 t, float2 samplePos, float2 sampleD)
{
	return
		mul(t,
			float4x4(
				tex2D(s0, samplePos - sampleD),
				tex2D(s0, samplePos),
				tex2D(s0, samplePos + sampleD),
				tex2D(s0, samplePos + sampleD + sampleD)
				)
			);
}

float4 main_bicubic_x(PS_INPUT input) : COLOR {
	float2 PixelPos = input.t0;
	float2 dd = frac(PixelPos);
	float2 ExactPixel = PixelPos - dd;
	float2 samplePos = ExactPixel * dxdy + dxdy05;

	return Sample(taps(dd.x), samplePos, dx);
}

float4 main_bicubic_y(PS_INPUT input) : COLOR {
	float2 PixelPos = input.t0;
	float2 dd = frac(PixelPos);
	float2 ExactPixel = PixelPos - dd;
	float2 samplePos = ExactPixel * dxdy + dxdy05;

	return Sample(taps(dd.y), samplePos, dy);
}
