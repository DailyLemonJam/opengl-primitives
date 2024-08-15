#version 330 core

in vec3 FragPos;

void main()
{
    //gl_FragColor = vec4(0.2f, FragPos.y * 2, 0.8f, 1.0);
    gl_FragColor = vec4(sin(FragPos.x), cos(FragPos.y), 0.8f, 1.0);
}
