struct Macro
{
	std::string m_name;
	std::string m_value;
};

struct gxToolMaterialConfig
{
	u32 m_hash;
	std::string m_debugName;
	std::string m_shaderNames[glShaderType_MAX];
	std::vector<Macro> m_macros;
};

struct glToolMaterialParameter
{
	enum Type
	{
		Type_Enum = 0,
		Type_Bool,
		Type_Int,
		Type_Float,
		Type_Sampler,

		Type_MAX
	};

	std::string m_name;

	Type m_type;
	int m_elemCount[2];	// X and Y elements count, e.g. float4x4
	int m_arraySize;	// array size (only 1-dimensional arrays supported)

	// ???
	bool m_isEditable;
	// ???
	bool m_isOverridableAtRuntime;
};

struct gxToolMaterial
{
	gxMaterialRenderState m_renderState;
	std::set<gxToolMaterialConfig> m_configs;
	std::vector<glToolMaterialParameter> m_params;
};