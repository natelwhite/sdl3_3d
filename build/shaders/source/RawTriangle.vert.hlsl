struct Input {
	uint VertexIndex : SV_VertexID;
};

struct Output {
	float4 Color : TEXCOORD0;
	float4 Position : SV_Position;
};

Output main(Input input) {
	Output output;
	float2 pos;
	switch(input.VertexIndex) {
		case 0:
		pos = float2(-1.0f, -1.0f);
		output.Color = float4(1.0f, 0.0f, 0.0f, 1.0f);
		break;
	case 1:
		pos = float2(1.0f, -1.0f);
		output.Color = float4(0.0f, 1.0f, 0.0f, 1.0f);
		break;
	case 2:
		pos = float2(0.0f, 1.0f);
		output.Color = float4(0.0f, 0.0f, 1.0f, 1.0f);
		break;
	}
	output.Position = float4(pos, 0.0f, 1.0f);
	return output;
}
