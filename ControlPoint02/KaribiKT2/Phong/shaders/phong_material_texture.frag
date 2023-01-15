#version 330 core

struct PositionalLight {
	vec3 Position;
	vec3 Ka;
	vec3 Kd;
	vec3 Ks;
	float Kc;
	float Kl;
	float Kq;
};

struct DirectionalLight {
	vec3 Position;
	vec3 Direction;
	vec3 Ka;
	vec3 Kd;
	vec3 Ks;
	float InnerCutOff;
	float OuterCutOff;
	float Kc;
	float Kl;
	float Kq;
};

struct Material {
	// NOTE(Jovan): Diffuse is used as ambient as well since the light source
	// defines the ambient colour
	sampler2D Kd;
	sampler2D Ks;
	float Shininess;
};

uniform PositionalLight uSunLight;
uniform PositionalLight uTorchLight1;
uniform PositionalLight uTorchLight2;
uniform PositionalLight uTorchLight3;

uniform DirectionalLight uLighthouseLight1;
uniform DirectionalLight uLighthouseLight2;

uniform DirectionalLight uDirLight;
uniform Material uMaterial;
uniform vec3 uViewPos;

in vec2 UV;
in vec3 vWorldSpaceFragment;
in vec3 vWorldSpaceNormal;

out vec4 FragColor;

void main() {
	vec3 ViewDirection = normalize(uViewPos - vWorldSpaceFragment);

	vec3 DirLightVector = normalize(-uDirLight.Direction);
	float DirDiffuse = max(dot(vWorldSpaceNormal, DirLightVector), 0.0f);
	vec3 DirReflectDirection = reflect(-DirLightVector, vWorldSpaceNormal);
	float DirSpecular = pow(max(dot(ViewDirection, DirReflectDirection), 0.0f), uMaterial.Shininess);
	vec3 DirAmbientColor = uDirLight.Ka * vec3(texture(uMaterial.Kd, UV));
	vec3 DirDiffuseColor = uDirLight.Kd * DirDiffuse * vec3(texture(uMaterial.Kd, UV));
	vec3 DirSpecularColor = uDirLight.Ks * DirSpecular * vec3(texture(uMaterial.Ks, UV));
	vec3 DirColor = DirAmbientColor + DirDiffuseColor + DirSpecularColor;

	// Sun
	vec3 PtLightVector = normalize(uSunLight.Position - vWorldSpaceFragment);
	float PtDiffuse = max(dot(vWorldSpaceNormal, PtLightVector), 0.0f);
	vec3 PtReflectDirection = reflect(-PtLightVector, vWorldSpaceNormal);
	float PtSpecular = pow(max(dot(ViewDirection, PtReflectDirection), 0.0f), uMaterial.Shininess);

	vec3 PtAmbientColor = uSunLight.Ka * vec3(texture(uMaterial.Kd, UV));
	vec3 PtDiffuseColor = PtDiffuse * uSunLight.Kd * vec3(texture(uMaterial.Kd, UV));
	vec3 PtSpecularColor = PtSpecular * uSunLight.Ks * vec3(texture(uMaterial.Ks, UV));

	float PtLightDistance = length(uSunLight.Position - vWorldSpaceFragment);
	float PtAttenuation = 1.0f / (uSunLight.Kc + uSunLight.Kl * PtLightDistance + uSunLight.Kq * (PtLightDistance * PtLightDistance));
	vec3 PtColorSun = PtAttenuation * (PtAmbientColor + PtDiffuseColor + PtSpecularColor);

	// Torch 1
	PtLightVector = normalize(uTorchLight1.Position - vWorldSpaceFragment);
	PtDiffuse = max(dot(vWorldSpaceNormal, PtLightVector), 0.0f);
	PtReflectDirection = reflect(-PtLightVector, vWorldSpaceNormal);
	PtSpecular = pow(max(dot(ViewDirection, PtReflectDirection), 0.0f), uMaterial.Shininess);

	PtAmbientColor = uTorchLight1.Ka * vec3(texture(uMaterial.Kd, UV));
	PtDiffuseColor = PtDiffuse * uTorchLight1.Kd * vec3(texture(uMaterial.Kd, UV));
	PtSpecularColor = PtSpecular * uTorchLight1.Ks * vec3(texture(uMaterial.Ks, UV));

	PtLightDistance = length(uTorchLight1.Position - vWorldSpaceFragment);
	PtAttenuation = 1.0f / (uTorchLight1.Kc + uTorchLight1.Kl * PtLightDistance + uTorchLight1.Kq * (PtLightDistance * PtLightDistance));
	vec3 PtColorTorch1 = PtAttenuation * (PtAmbientColor + PtDiffuseColor + PtSpecularColor);

	
	// Torch 2
	PtLightVector = normalize(uTorchLight2.Position - vWorldSpaceFragment);
	PtDiffuse = max(dot(vWorldSpaceNormal, PtLightVector), 0.0f);
	PtReflectDirection = reflect(-PtLightVector, vWorldSpaceNormal);
	PtSpecular = pow(max(dot(ViewDirection, PtReflectDirection), 0.0f), uMaterial.Shininess);

	PtAmbientColor = uTorchLight2.Ka * vec3(texture(uMaterial.Kd, UV));
	PtDiffuseColor = PtDiffuse * uTorchLight2.Kd * vec3(texture(uMaterial.Kd, UV));
	PtSpecularColor = PtSpecular * uTorchLight2.Ks * vec3(texture(uMaterial.Ks, UV));

	PtLightDistance = length(uTorchLight2.Position - vWorldSpaceFragment);
	PtAttenuation = 1.0f / (uTorchLight2.Kc + uTorchLight2.Kl * PtLightDistance + uTorchLight2.Kq * (PtLightDistance * PtLightDistance));
	vec3 PtColorTorch2 = PtAttenuation * (PtAmbientColor + PtDiffuseColor + PtSpecularColor);

	// Torch 3
	PtLightVector = normalize(uTorchLight3.Position - vWorldSpaceFragment);
	PtDiffuse = max(dot(vWorldSpaceNormal, PtLightVector), 0.0f);
	PtReflectDirection = reflect(-PtLightVector, vWorldSpaceNormal);
	PtSpecular = pow(max(dot(ViewDirection, PtReflectDirection), 0.0f), uMaterial.Shininess);

	PtAmbientColor = uTorchLight3.Ka * vec3(texture(uMaterial.Kd, UV));
	PtDiffuseColor = PtDiffuse * uTorchLight3.Kd * vec3(texture(uMaterial.Kd, UV));
	PtSpecularColor = PtSpecular * uTorchLight3.Ks * vec3(texture(uMaterial.Ks, UV));

	PtLightDistance = length(uTorchLight3.Position - vWorldSpaceFragment);
	PtAttenuation = 1.0f / (uTorchLight3.Kc + uTorchLight3.Kl * PtLightDistance + uTorchLight3.Kq * (PtLightDistance * PtLightDistance));
	vec3 PtColorTorch3 = PtAttenuation * (PtAmbientColor + PtDiffuseColor + PtSpecularColor);


	// LighthouseLight1
	vec3 SpotlightVector1 = normalize(uLighthouseLight1.Position - vWorldSpaceFragment);

	float SpotDiffuse1 = max(dot(vWorldSpaceNormal, SpotlightVector1), 0.0f);
	vec3 SpotReflectDirection1 = reflect(-SpotlightVector1, vWorldSpaceNormal);
	float SpotSpecular1 = pow(max(dot(ViewDirection, SpotReflectDirection1), 0.0f), uMaterial.Shininess);

	vec3 SpotAmbientColor1 = uLighthouseLight1.Ka * vec3(texture(uMaterial.Kd, UV));
	vec3 SpotDiffuseColor1 = SpotDiffuse1 * uLighthouseLight1.Kd * vec3(texture(uMaterial.Kd, UV));
	vec3 SpotSpecularColor1 = SpotSpecular1 * uLighthouseLight1.Ks * vec3(texture(uMaterial.Ks, UV));

	float SpotlightDistance1 = length(uLighthouseLight1.Position - vWorldSpaceFragment);
	float SpotAttenuation1 = 1.0f / (uLighthouseLight1.Kc + uLighthouseLight1.Kl * SpotlightDistance1 + uLighthouseLight1.Kq * (SpotlightDistance1 * SpotlightDistance1));

	float Theta1 = dot(SpotlightVector1, normalize(-uLighthouseLight1.Direction));
	float Epsilon1 = uLighthouseLight1.InnerCutOff - uLighthouseLight1.OuterCutOff;
	float SpotIntensity1 = clamp((Theta1 - uLighthouseLight1.OuterCutOff) / Epsilon1, 0.0f, 1.0f);
	vec3 SpotColor1 = SpotIntensity1 * SpotAttenuation1 * (SpotAmbientColor1 + SpotDiffuseColor1 + SpotSpecularColor1);

	// LighthouseLight2
	vec3 SpotlightVector2 = normalize(uLighthouseLight2.Position - vWorldSpaceFragment);

	float SpotDiffuse2 = max(dot(vWorldSpaceNormal, SpotlightVector2), 0.0f);
	vec3 SpotReflectDirection2 = reflect(-SpotlightVector2, vWorldSpaceNormal);
	float SpotSpecular2 = pow(max(dot(ViewDirection, SpotReflectDirection2), 0.0f), uMaterial.Shininess);

	vec3 SpotAmbientColor2 = uLighthouseLight2.Ka * vec3(texture(uMaterial.Kd, UV));
	vec3 SpotDiffuseColor2 = SpotDiffuse2 * uLighthouseLight2.Kd * vec3(texture(uMaterial.Kd, UV));
	vec3 SpotSpecularColor2 = SpotSpecular2 * uLighthouseLight2.Ks * vec3(texture(uMaterial.Ks, UV));

	float SpotlightDistance2 = length(uLighthouseLight2.Position - vWorldSpaceFragment);
	float SpotAttenuation2 = 1.0f / (uLighthouseLight2.Kc + uLighthouseLight2.Kl * SpotlightDistance2 + uLighthouseLight2.Kq * (SpotlightDistance2 * SpotlightDistance2));

	float Theta2 = dot(SpotlightVector2, normalize(-uLighthouseLight2.Direction));
	float Epsilon2 = uLighthouseLight2.InnerCutOff - uLighthouseLight2.OuterCutOff;
	float SpotIntensity2 = clamp((Theta2 - uLighthouseLight2.OuterCutOff) / Epsilon2, 0.0f, 1.0f);
	vec3 SpotColor2 = SpotIntensity2 * SpotAttenuation2 * (SpotAmbientColor2 + SpotDiffuseColor2 + SpotSpecularColor2);


	
	vec3 FinalColor = DirColor + PtColorSun + PtColorTorch1 + PtColorTorch2 + PtColorTorch3 + SpotColor1 + SpotColor2;
	FragColor = vec4(FinalColor, 1.0f);
}