#pragma once

#include <cstdio>
#include <memory>
#include <cstdlib>
#include <map>
#include <string>
#include <cstring>
#include <vector>
#include <algorithm>
#include <cmath>
#include <iostream>

#include <ruis/config.hpp>

//-----------------------------------------------------------------------------
// Alias|Wavefront OBJ file loader.
//
// This OBJ file loader contains the following restrictions:
// 1. Group information is ignored. Faces are grouped based on the material
//    that each face uses.
// 2. Object information is ignored. This loader will merge everything into a
//    single object.
// 3. The MTL file must be located in the same directory as the OBJ file. If
//    it isn't then the MTL file will fail to load and a default material is
//    used instead.
// 4. This loader triangulates all polygonal faces during importing.
//-----------------------------------------------------------------------------


class ModelOBJ
{
public:

    //unsigned int vaoHandle;
    unsigned int vboHandles[2];
    void buildVBOs();
	void createVBOs();
    void render();

    struct Material
    {
        float ambient[4];
        float diffuse[4];
        float specular[4];
        float shininess;        // [0 = min shininess, 1 = max shininess]
        float alpha;            // [0 = fully transparent, 1 = fully opaque]

        std::string name;
        std::string colorMapFilename;
        std::string bumpMapFilename;
    };

    struct Vertex
    {
        //float position[3];
        float position[4];
        float texCoord[2];
        float normal[3];
        float tangent[4];
        float bitangent[3];
    };

    struct Mesh
    {
        int startIndex;
        int triangleCount;
        const Material *pMaterial;
    };

    ModelOBJ();
    ~ModelOBJ();

    void destroy();
    bool import(const std::string& filename, bool rebuildNormals = false);
    void normalize(float scaleTo = 1.0f, bool center = true);
    void reverseWinding();

    // Getter methods.

    void getCenter(float &x, float &y, float &z) const;
    float getWidth() const;
    float getHeight() const;
    float getLength() const;
    float getRadius() const;

    const int* getIndexBuffer() const;
    const uint16_t* getShortIndexBufferPtr() const;
    int getIndexSize() const;

    const Material &getMaterial(int i) const;
    const Mesh &getMesh(int i) const;

    int getNumberOfIndices() const;
    int getNumberOfMaterials() const;
    int getNumberOfMeshes() const;
    int getNumberOfTriangles() const;
    int getNumberOfVertices() const;

    const std::string &getPath() const;

    const Vertex &getVertex(int i) const;
    const Vertex *getVertexBuffer() const;
    int getVertexSize() const;

    bool hasNormals() const;
    bool hasPositions() const;
    bool hasTangents() const;
    bool hasTextureCoords() const;

    void bounds(float center[3], float &width, float &height,
        float &length, float &radius) const;

    const std::vector<ruis::vec4>& getPositionsBuffer() const {return positions;}
    const std::vector<ruis::vec2>& getTextureCoordsBuffer() const {return textureCoords;}
    const std::vector<ruis::vec3>& getNormalsBuffer() const {return normals;}
    const std::vector<ruis::vec4>& getTangentsBuffer() const {return tangents;}
    const std::vector<ruis::vec4>& getBitangentsBuffer() const {return bitangents;}

    const std::vector<uint16_t>& getShortIndexBuffer() const {return m_shortIndexBuffer;}
private:
    void addTrianglePos(int index, int material,
        int v0, int v1, int v2);
    void addTrianglePosNormal(int index, int material,
        int v0, int v1, int v2,
        int vn0, int vn1, int vn2);
    void addTrianglePosTexCoord(int index, int material,
        int v0, int v1, int v2,
        int vt0, int vt1, int vt2);
    void addTrianglePosTexCoordNormal(int index, int material,
        int v0, int v1, int v2,
        int vt0, int vt1, int vt2,
        int vn0, int vn1, int vn2);
    int addVertex(int hash, const Vertex *pVertex);

    void buildMeshes();
    void generateNormals();
    void generateTangents();
    void importGeometryFirstPass(FILE *pFile);
    void importGeometrySecondPass(FILE *pFile);
    bool importMaterials(const char *pszFilename);
    void scale(float scaleFactor, float offset[3]);

    void buildShortIndexBuffer();

    bool m_hasPositions;
    bool m_hasTextureCoords;
    bool m_hasNormals;
    bool m_hasTangents;

    int m_numberOfVertexCoords;
    int m_numberOfTextureCoords;
    int m_numberOfNormals;
    int m_numberOfTriangles;
    int m_numberOfMaterials;
    int m_numberOfMeshes;

    float m_center[3];
    float m_width;
    float m_height;
    float m_length;
    float m_radius;

    std::string m_directoryPath;

    std::vector<Mesh> m_meshes;
    std::vector<Material> m_materials;
    std::vector<int> m_attributeBuffer;

    std::vector<Vertex> m_vertexBuffer;
    std::vector<int> m_indexBuffer;
    std::vector<uint16_t> m_shortIndexBuffer;

    std::vector<float> m_vertexCoords;
    std::vector<float> m_textureCoords;
    std::vector<float> m_normals;

    /// export-data
    std::vector<ruis::vec4> positions;
    std::vector<ruis::vec2> textureCoords;
    std::vector<ruis::vec3> normals;
    std::vector<ruis::vec4> tangents;
    std::vector<ruis::vec4> bitangents;

    std::map<std::string, int> m_materialCache;
    std::map<int, std::vector<int> > m_vertexCache;
};

//-----------------------------------------------------------------------------

inline void ModelOBJ::getCenter(float &x, float &y, float &z) const
{ x = m_center[0]; y = m_center[1]; z = m_center[2]; }

inline float ModelOBJ::getWidth() const
{ return m_width; }

inline float ModelOBJ::getHeight() const
{ return m_height; }

inline float ModelOBJ::getLength() const
{ return m_length; }

inline float ModelOBJ::getRadius() const
{ return m_radius; }

inline const int* ModelOBJ::getIndexBuffer() const
{ return &m_indexBuffer[0]; }

inline const uint16_t* ModelOBJ::getShortIndexBufferPtr() const
{ return &m_shortIndexBuffer[0]; }


inline int ModelOBJ::getIndexSize() const
{ return static_cast<int>(sizeof(int)); }

inline const ModelOBJ::Material &ModelOBJ::getMaterial(int i) const
{ return m_materials[i]; }

inline const ModelOBJ::Mesh &ModelOBJ::getMesh(int i) const
{ return m_meshes[i]; }

inline int ModelOBJ::getNumberOfIndices() const
{ return m_numberOfTriangles * 3; }

inline int ModelOBJ::getNumberOfMaterials() const
{ return m_numberOfMaterials; }

inline int ModelOBJ::getNumberOfMeshes() const
{ return m_numberOfMeshes; }

inline int ModelOBJ::getNumberOfTriangles() const
{ return m_numberOfTriangles; }

inline int ModelOBJ::getNumberOfVertices() const
{ return static_cast<int>(m_vertexBuffer.size()); }

inline const std::string &ModelOBJ::getPath() const
{ return m_directoryPath; }

inline const ModelOBJ::Vertex &ModelOBJ::getVertex(int i) const
{ return m_vertexBuffer[i]; }

inline const ModelOBJ::Vertex *ModelOBJ::getVertexBuffer() const
{ return &m_vertexBuffer[0]; }

inline int ModelOBJ::getVertexSize() const
{ return static_cast<int>(sizeof(Vertex)); }

inline bool ModelOBJ::hasNormals() const
{ return m_hasNormals; }

inline bool ModelOBJ::hasPositions() const
{ return m_hasPositions; }

inline bool ModelOBJ::hasTangents() const
{ return m_hasTangents; }

inline bool ModelOBJ::hasTextureCoords() const
{ return m_hasTextureCoords; }

