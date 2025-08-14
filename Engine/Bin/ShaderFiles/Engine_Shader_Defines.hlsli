
sampler DefaultSampler = sampler_state
{
    filter = min_mag_mip_linear;
    AddressU = wrap;
    AddressV = wrap;
};


sampler PointSampler = sampler_state
{
    filter = min_mag_mip_point;
    AddressU = wrap;
    AddressV = wrap;
};

sampler LinearClampSampler = sampler_state
{
    filter = min_mag_mip_linear;
    AddressU = clamp;
    AddressV = clamp;
};
sampler LinearWrapSampler = sampler_state
{
    Filter = min_mag_mip_linear;
    AddressU = wrap;
    AddressV = wrap;
};

sampler MirrorSampler = sampler_state
{
    filter = min_mag_mip_linear;
    AddressU = mirror;
    AddressV = mirror;
};

RasterizerState RS_Default
{
    FillMode = Solid;
    CullMode = back;
};


RasterizerState RS_Wireframe
{
    FillMode = Wireframe;
    CullMode = back;
    
};

RasterizerState RS_Cull_Front
{
    FillMode = Solid;
    CullMode = front;
};

RasterizerState RS_Cull_None
{
    FillMode = Solid;
    CullMode = NONE;
    FrontCounterClockwise = FALSE;
};


DepthStencilState DSS_Default
{
    DepthEnable = true;
    DepthWriteMask = all;
    DepthFunc = less_equal;
};

DepthStencilState DSS_ReadOnlyDepth
{
    DepthEnable = true;
    DepthWriteMask = ZERO;
};

DepthStencilState DSS_None
{
    DepthEnable = false;
    DepthWriteMask = zero;    
};

BlendState BS_Default
{
    BlendEnable[0] = false;

};

BlendState BS_AlphaBlend
{
    BlendEnable[0] = true;
    SrcBlend = Src_Alpha;
    DestBlend = Inv_Src_Alpha;
    BlendOp = Add;
};
BlendState BS_OneBlend
{
    BlendEnable[0] = true;
    BlendEnable[1] = true;
    SrcBlend = one;
    DestBlend = one;
    BlendOp = Add;
};

BlendState BS_SoftAdd
{
    BlendEnable[0] = true;
    BlendEnable[1] = true;
    SrcBlend = One;
    DestBlend = Inv_Src_Alpha;
    BlendOp = Add;
};

BlendState BS_WBOIT
{
    // RTV0 : Accumulation (additive)
    BlendEnable[0] = true;
    SrcBlend[0] = One;
    DestBlend[0] = One;
    BlendOp[0] = Add;

    // RTV1 : Revealage (multiplicative)
    BlendEnable[1] = true;
    SrcBlend[1] = Zero;
    DestBlend[1] = Inv_Src_Alpha; // dest * (1 - srcAlpha)
    BlendOp[1] = Add;
    SrcBlendAlpha[1] = Zero;
    DestBlendAlpha[1] = Inv_Src_Alpha;
    BlendOpAlpha[1] = Add;
    //RenderTargetWriteMask[1] = RED;
    // Revealage는 이미 있던 알파 값에 1-srcAlpha만 계속해서 들고만 있으면 되므로
    // SrcBlend는 무시한 후 DestBlend값을 남겨두는 의미로 BlendOP를 Add로 함.

    // RTV2 : Emissive (Additive)
    BlendEnable[2] = true;
    SrcBlend[2] = One;
    DestBlend[2] = One;
    BlendOp[2] = Add;
    // Emissive를 저장하긴 해야해서 명시적으로 블렌드 스테이트 지정함

    // RTV3 : Distortion (Additive)
    BlendEnable[3] = true;
    SrcBlend[3] = One;
    DestBlend[3] = One; // dest * (1 - srcAlpha)
    BlendOp[3] = Add;
    SrcBlendAlpha[3] = One;
    DestBlendAlpha[3] = Inv_Src_Alpha;
    
};
