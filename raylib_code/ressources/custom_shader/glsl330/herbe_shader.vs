#version 330

// Input vertex attributes
in vec3 vertexPosition;
in vec2 vertexTexCoord;
in vec3 vertexNormal;
in vec4 vertexColor;

// Input uniform values
uniform mat4 mvp;
uniform mat4 matModel;
uniform mat4 matNormal;
uniform float wind_speed;
uniform float wind_strength;
uniform float wind_texture_tile_size;
uniform float wind_vertical_strength;
uniform vec2 wind_horizontal_direction;
uniform sampler2D wind_noise;
uniform float time;

// Output vertex attributes (to fragment shader)
out vec3 fragPosition;
out vec2 fragTexCoord;
out vec4 fragColor;
out vec3 fragNormal;

void main() {
    vec3 world_vert = vec3(matModel * vec4(vertexPosition, 1.0));
    vec2 normalized_wind_direction = normalize(wind_horizontal_direction);
    vec2 world_uv = world_vert.xz / wind_texture_tile_size + normalized_wind_direction * time * wind_speed;
    float displacement_affect = (1.0 - vertexTexCoord.y);
    float wind_noise_intensity = (texture(wind_noise, world_uv).r - 0.5);
    vec3 bump_wind = vec3(
        wind_noise_intensity * normalized_wind_direction.x * wind_strength,
        (1.0 - wind_noise_intensity) * wind_vertical_strength,
        wind_noise_intensity * normalized_wind_direction.y * wind_strength
    );
    vec3 displacedPosition = vertexPosition + bump_wind * displacement_affect;
    fragPosition = vec3(matModel * vec4(displacedPosition, 1.0));
    fragTexCoord = vertexTexCoord;
    fragColor = vertexColor;
    fragNormal = normalize(vec3(matNormal * vec4(vertexNormal, 1.0)));
    gl_Position = mvp * vec4(displacedPosition, 1.0);
}