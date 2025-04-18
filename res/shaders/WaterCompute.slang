const static float PI = 3.1415926535897932f;
// FFT and JONSWAP Implementation largely referenced from https://github.com/gasgiant/FFT-Ocean/
struct SpectrumParameters {
	float scale;
	float angle;
	float spreadBlend;
	float swell;
	float alpha;
	float peakOmega;
	float gamma;
	float shortWavesFade;
};

struct ComputeParams
{
    float frameTime, deltaTime, gravity, repeatTime, depth, lowCutoff, highCutoff;
    int seed;
    float2 lambda;
    uint N, lengthScale0, lengthScale1, lengthScale2, lengthScale3;
    float foamBias, foamDecayRate, foamAdd, foamThreshold;
    RWTexture2DArray<float4> spectrumTextures, initialSpectrumTextures, displacementTextures;
    RWTexture2DArray<float4> slopeTexture;
    RWTexture2D<half> boyancyData;
    StructuredBuffer<SpectrumParameters> spectrums;
    SamplerState linear_repeat_sampler;
};
layout(set = 0)
ParameterBlock<ComputeParams> pParams;

float2 ComplexMult(float2 a, float2 b) {
    return float2(a.x * b.x - a.y * b.y, a.x * b.y + a.y * b.x);
}

float2 EulerFormula(float x) {
    return float2(cos(x), sin(x));
}

float hash(uint n) {
    // integer hash copied from Hugo Elias
    n = (n << 13U) ^ n;
    n = n * (n * n * 15731U + 0x789221U) + (0x1376312589U);
    return float(n & uint(0x7fffffffU)) / float(0x7fffffff);
}

float2 UniformToGaussian(float u1, float u2) {
    float R = sqrt(-2.0f * log(u1));
    float theta = 2.0f * PI * u2;

    return float2(R * cos(theta), R * sin(theta));
}


float Dispersion(float kMag) {
    return sqrt(pParams.gravity * kMag * tanh(min(kMag * pParams.depth, 20)));
}

float DispersionDerivative(float kMag) {
    float th = tanh(min(kMag * pParams.depth, 20));
    float ch = cosh(kMag * pParams.depth);
    return pParams.gravity * (pParams.depth * kMag / ch / ch + th) / Dispersion(kMag) / 2.0f;
}

float NormalizationFactor(float s) {
    float s2 = s * s;
    float s3 = s2 * s;
    float s4 = s3 * s;
    if (s < 5) return -0.000564f * s4 + 0.00776f * s3 - 0.044f * s2 + 0.192f * s + 0.163f;
    else return -4.80e-08f * s4 + 1.07e-05f * s3 - 9.53e-04f * s2 + 5.90e-02f * s + 3.93e-01f;
}

float DonelanBannerBeta(float x) {
	if (x < 0.95f) return 2.61f * pow(abs(x), 1.3f);
	if (x < 1.6f) return 2.28f * pow(abs(x), -1.3f);

	float p = -0.4f + 0.8393f * exp(-0.567f * log(x * x));
	return pow(10.0f, p);
}

float DonelanBanner(float theta, float omega, float peakOmega) {
	float beta = DonelanBannerBeta(omega / peakOmega);
	float sech = 1.0f / cosh(beta * theta);
	return beta / 2.0f / tanh(beta * 3.1416f) * sech * sech;
}

float Cosine2s(float theta, float s) {
	return NormalizationFactor(s) * pow(abs(cos(0.5f * theta)), 2.0f * s);
}

float SpreadPower(float omega, float peakOmega) {
	if (omega > peakOmega)
		return 9.77f * pow(abs(omega / peakOmega), -2.5f);
	else
		return 6.97f * pow(abs(omega / peakOmega), 5.0f);
}

float DirectionSpectrum(float theta, float omega, SpectrumParameters spectrum) {
	float s = SpreadPower(omega, spectrum.peakOmega) + 16 * tanh(min(omega / spectrum.peakOmega, 20)) * spectrum.swell * spectrum.swell;
	return lerp(2.0f / 3.1415f * cos(theta) * cos(theta), Cosine2s(theta - spectrum.angle, s), spectrum.spreadBlend);
}

float TMACorrection(float omega) {
	float omegaH = omega * sqrt(pParams.depth / pParams.gravity);
	if (omegaH <= 1.0f)
		return 0.5f * omegaH * omegaH;
	if (omegaH < 2.0f)
		return 1.0f - 0.5f * (2.0f - omegaH) * (2.0f - omegaH);

	return 1.0f;
}

float JONSWAP(float omega, SpectrumParameters spectrum) {
	float sigma = (omega <= spectrum.peakOmega) ? 0.07f : 0.09f;

	float r = exp(-(omega - spectrum.peakOmega) * (omega - spectrum.peakOmega) / 2.0f / sigma / sigma / spectrum.peakOmega / spectrum.peakOmega);
	
	float oneOverOmega = 1.0f / omega;
	float peakOmegaOverOmega = spectrum.peakOmega / omega;
	return spectrum.scale * TMACorrection(omega) * spectrum.alpha * pParams.gravity * pParams.gravity
		* oneOverOmega * oneOverOmega * oneOverOmega * oneOverOmega * oneOverOmega
		* exp(-1.25f * peakOmegaOverOmega * peakOmegaOverOmega * peakOmegaOverOmega * peakOmegaOverOmega)
		* pow(abs(spectrum.gamma), r);
}

float ShortWavesFade(float kLength, SpectrumParameters spectrum) {
	return exp(-spectrum.shortWavesFade * spectrum.shortWavesFade * kLength * kLength);
}

[numthreads(8,8,1)]
void CS_InitializeSpectrum(uint3 id : SV_DISPATCHTHREADID) {
    uint seed = id.x + pParams.N * id.y + pParams.N;
    seed += pParams.seed;

    float lengthScales[4] = { pParams.lengthScale0, pParams.lengthScale1, pParams.lengthScale2, pParams.lengthScale3 };

    for (uint i = 0; i < 4; ++i) {
        float halfN = pParams.N / 2.0f;

        float deltaK = 2.0f * PI / lengthScales[i];
        float2 K = (id.xy - halfN) * deltaK;
        float kLength = length(K);

        seed += i + uint(hash(seed) * 10);
        float4 uniformRandSamples = float4(hash(seed), hash(seed * 2), hash(seed * 3), hash(seed * 4));
        float2 gauss1 = UniformToGaussian(uniformRandSamples.x, uniformRandSamples.y);
        float2 gauss2 = UniformToGaussian(uniformRandSamples.z, uniformRandSamples.w);

        if (pParams.lowCutoff <= kLength && kLength <= pParams.highCutoff) {
            float kAngle = atan2(K.y, K.x);
            float omega = Dispersion(kLength);

            float dOmegadk = DispersionDerivative(kLength);

            float spectrum = JONSWAP(omega, pParams.spectrums[i * 2]) * DirectionSpectrum(kAngle, omega, pParams.spectrums[i * 2]) * ShortWavesFade(kLength, pParams.spectrums[i * 2]);
            
            if (pParams.spectrums[i * 2 + 1].scale > 0)
                spectrum += JONSWAP(omega, pParams.spectrums[i * 2 + 1]) * DirectionSpectrum(kAngle, omega, pParams.spectrums[i * 2 + 1]) * ShortWavesFade(kLength, pParams.spectrums[i * 2 + 1]);
            
            pParams.initialSpectrumTextures[uint3(id.xy, i)] = float4(float2(gauss2.x, gauss1.y) * sqrt(2 * spectrum * abs(dOmegadk) / kLength * deltaK * deltaK), 0.0f, 0.0f);
        } else {
            pParams.initialSpectrumTextures[uint3(id.xy, i)] = 0.0f;
        }
    }
}

[numthreads(8,8,1)]
void CS_PackSpectrumConjugate(uint3 id : SV_DISPATCHTHREADID) {
    for (uint i = 0; i < 4; ++i) {
        float2 h0 = pParams.initialSpectrumTextures[uint3(id.xy, i)].rg;
        float2 h0conj = pParams.initialSpectrumTextures[uint3((pParams.N - id.x ) % pParams.N, (pParams.N - id.y) % pParams.N, i)].rg;

        pParams.initialSpectrumTextures[uint3(id.xy, i)] = float4(h0, h0conj.x, -h0conj.y);
    }
}

[numthreads(8, 8, 1)]
void CS_UpdateSpectrumForFFT(uint3 id : SV_DISPATCHTHREADID) {
    
    float lengthScales[4] = { pParams.lengthScale0, pParams.lengthScale1, pParams.lengthScale2, pParams.lengthScale3 };

    for (int i = 0; i < 4; ++i) {
        float4 initialSignal = pParams.initialSpectrumTextures[uint3(id.xy, i)];
        float2 h0 = initialSignal.xy;
        float2 h0conj = initialSignal.zw;

        float halfN = pParams.N / 2.0f;
        float2 K = (id.xy - halfN) * 2.0f * PI / lengthScales[i];
        float kMag = length(K);
        float kMagRcp = rcp(kMag);

        if (kMag < 0.0001f) {
            kMagRcp = 1.0f;
        }

        float w_0 = 2.0f * PI / pParams.repeatTime;
        float dispersion = floor(sqrt(pParams.gravity * kMag) / w_0) * w_0 * pParams.frameTime;

        float2 exponent = EulerFormula(dispersion);

        float2 htilde = ComplexMult(h0, exponent) + ComplexMult(h0conj, float2(exponent.x, -exponent.y));
        float2 ih = float2(-htilde.y, htilde.x);

        float2 displacementX = ih * K.x * kMagRcp;
        float2 displacementY = htilde;
        float2 displacementZ = ih * K.y * kMagRcp;

        float2 displacementX_dx = -htilde * K.x * K.x * kMagRcp;
        float2 displacementY_dx = ih * K.x;
        float2 displacementZ_dx = -htilde * K.x * K.y * kMagRcp;

        float2 displacementY_dz = ih * K.y;
        float2 displacementZ_dz = -htilde * K.y * K.y * kMagRcp;

        float2 htildeDisplacementX = float2(displacementX.x - displacementZ.y, displacementX.y + displacementZ.x);
        float2 htildeDisplacementZ = float2(displacementY.x - displacementZ_dx.y, displacementY.y + displacementZ_dx.x);
        
        float2 htildeSlopeX = float2(displacementY_dx.x - displacementY_dz.y, displacementY_dx.y + displacementY_dz.x);
        float2 htildeSlopeZ = float2(displacementX_dx.x - displacementZ_dz.y, displacementX_dx.y + displacementZ_dz.x);

        pParams.spectrumTextures[uint3(id.xy, i * 2)] = float4(htildeDisplacementX, htildeDisplacementZ);
        pParams.spectrumTextures[uint3(id.xy, i * 2 + 1)] = float4(htildeSlopeX, htildeSlopeZ);
    }
}

#define SIZE 1024
#define LOG_SIZE 10

groupshared float4 fftGroupBuffer[2][SIZE];

void ButterflyValues(uint step, uint index, out uint2 indices, out float2 twiddle) {
    const float twoPi = 6.28318530718;
    uint b = SIZE >> (step + 1);
    uint w = b * (index / b);
    uint i = (w + index) % SIZE;
    sincos(-twoPi / SIZE * w, twiddle.y, twiddle.x);

    //This is what makes it the inverse FFT
    twiddle.y = -twiddle.y;
    indices = uint2(i, i + b);
}

float4 FFT(uint threadIndex, float4 input) {
    fftGroupBuffer[0][threadIndex] = input;
    GroupMemoryBarrierWithGroupSync();
    bool flag = false;

    [unroll]
    for (uint step = 0; step < LOG_SIZE; ++step) {
        uint2 inputsIndices;
        float2 twiddle;
        ButterflyValues(step, threadIndex, inputsIndices, twiddle);

        float4 v = fftGroupBuffer[uint(flag)][inputsIndices.y];
        fftGroupBuffer[uint(!flag)][threadIndex] = fftGroupBuffer[uint(flag)][inputsIndices.x] + float4(ComplexMult(twiddle, v.xy), ComplexMult(twiddle, v.zw));

        flag = !flag;
        GroupMemoryBarrierWithGroupSync();
    }

    return fftGroupBuffer[uint(flag)][threadIndex];
}

[numthreads(SIZE, 1, 1)]
void CS_HorizontalFFT(uint3 id : SV_DISPATCHTHREADID) {
    for (int i = 0; i < 8; ++i) {
        pParams.spectrumTextures[uint3(id.xy, i)] = FFT(id.x, pParams.spectrumTextures[uint3(id.xy, i)]);
    }
}

[numthreads(SIZE, 1, 1)]
void CS_VerticalFFT(uint3 id : SV_DISPATCHTHREADID) {
    for (int i = 0; i < 8; ++i) {
        pParams.spectrumTextures[uint3(id.yx, i)] = FFT(id.x, pParams.spectrumTextures[uint3(id.yx, i)]);
    }
}

float4 Permute(float4 data, float3 id) {
    return data * (1.0f - 2.0f * ((id.x + id.y) % 2));
}

[numthreads(8, 8, 1)]
void CS_AssembleMaps(uint3 id : SV_DISPATCHTHREADID) {
    for (int i = 0; i < 4; ++i) {
        float4 htildeDisplacement = Permute(pParams.spectrumTextures[uint3(id.xy, i * 2)], id);
        float4 htildeSlope = Permute(pParams.spectrumTextures[uint3(id.xy, i * 2 + 1)], id);

        float2 dxdz = htildeDisplacement.rg;
        float2 dydxz = htildeDisplacement.ba;
        float2 dyxdyz = htildeSlope.rg;
        float2 dxxdzz = htildeSlope.ba;
        
        float jacobian = (1.0f + pParams.lambda.x * dxxdzz.x) * (1.0f + pParams.lambda.y * dxxdzz.y) - pParams.lambda.x * pParams.lambda.y * dydxz.y * dydxz.y;

        float3 displacement = float3(pParams.lambda.x * dxdz.x, dydxz.x, pParams.lambda.y * dxdz.y);

        float2 slopes = dyxdyz.xy / (1 + abs(dxxdzz * pParams.lambda));
        float covariance = slopes.x * slopes.y;

        float foam = pParams.displacementTextures[uint3(id.xy, i)].a;
        foam *= exp(-pParams.foamDecayRate);
        foam = saturate(foam);

        float biasedJacobian = max(0.0f, -(jacobian - pParams.foamBias));

        if (biasedJacobian > pParams.foamThreshold)
            foam += pParams.foamAdd * biasedJacobian;


        pParams.displacementTextures[uint3(id.xy, i)] = float4(displacement, foam);
        pParams.slopeTexture[uint3(id.xy, i)] = float4(slopes, 0, 0);

        if (i == 0) {
            pParams.boyancyData[id.xy] = half(displacement.y);
        }
    }
}
