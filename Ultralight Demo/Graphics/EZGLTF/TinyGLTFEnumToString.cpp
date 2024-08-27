#include "PCH.h"
#include "TinyGLTFEnumToString.h"

//With header only files, it is common that a single compilation unit will require these defines to pull
//in the implementations one time and prevent redefinition errors during compilation
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "tiny_gltf/tiny_gltf.h"

std::string TinyGLTFComponentTypeToString(int tinyGltfEnum)
{
    switch (tinyGltfEnum)
    {
    case TINYGLTF_COMPONENT_TYPE_BYTE:
        return "TINYGLTF_COMPONENT_TYPE_BYTE";
    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
        return "TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE";
    case TINYGLTF_COMPONENT_TYPE_SHORT:
        return "TINYGLTF_COMPONENT_TYPE_SHORT";
    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
        return "TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT";
    case TINYGLTF_COMPONENT_TYPE_INT:
        return "TINYGLTF_COMPONENT_TYPE_INT";
    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
        return "TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT";
    case TINYGLTF_COMPONENT_TYPE_FLOAT:
        return "TINYGLTF_COMPONENT_TYPE_FLOAT";
    case TINYGLTF_COMPONENT_TYPE_DOUBLE:
        return "TINYGLTF_COMPONENT_TYPE_DOUBLE";
    default:
        return "?";
    }
}

std::string TinyGLTFModeToString(int modeEnum)
{
    switch (modeEnum)
    {
    case TINYGLTF_MODE_POINTS:
        return "TINYGLTF_MODE_POINTS";
    case TINYGLTF_MODE_LINE:
        return "TINYGLTF_MODE_LINE";
    case TINYGLTF_MODE_LINE_LOOP:
        return "TINYGLTF_MODE_LINE_LOOP";
    case TINYGLTF_MODE_LINE_STRIP:
        return "TINYGLTF_MODE_LINE_STRIP";
    case TINYGLTF_MODE_TRIANGLES:
        return "TINYGLTF_MODE_TRIANGLES";
    case TINYGLTF_MODE_TRIANGLE_STRIP:
        return "TINYGLTF_MODE_TRIANGLE_STRIP";
    case TINYGLTF_MODE_TRIANGLE_FAN:
        return "TINYGLTF_MODE_TRIANGLE_FAN";
    default:
        return "?";
    }
}

std::string TinyGLTFTypeToString(int tinyGltfEnum)
{
    switch (tinyGltfEnum)
    {
    case TINYGLTF_TYPE_VEC2:
        return "TINYGLTF_TYPE_VEC2";
    case TINYGLTF_TYPE_VEC3:
        return "TINYGLTF_TYPE_VEC3";
    case TINYGLTF_TYPE_VEC4:
        return "TINYGLTF_TYPE_VEC4";
    case TINYGLTF_TYPE_MAT2:
        return "TINYGLTF_TYPE_MAT2";
    case TINYGLTF_TYPE_MAT3:
        return "TINYGLTF_TYPE_MAT3";
    case TINYGLTF_TYPE_MAT4:
        return "TINYGLTF_TYPE_MAT4";
    case TINYGLTF_TYPE_SCALAR:
        return "TINYGLTF_TYPE_SCALAR";
    case TINYGLTF_TYPE_VECTOR:
        return "TINYGLTF_TYPE_VECTOR";
    case TINYGLTF_TYPE_MATRIX:
        return "TINYGLTF_TYPE_MATRIX";
    default:
        return "?";
    }

}