//- -----------------------------------------------------
//
// The methods normalize() and scale() are based on source code from
// http://www.mvps.org/directx/articles/scalemesh9.htm.
//
// The addVertex() method is based on source code from the Direct3D MeshFromOBJ
// sample found in the DirectX SDK.
//
// The generateTangents() method is based on public source code from
// http://www.terathon.com/code/tangent.php.
//
// The importGeometryFirstPass(), importGeometrySecondPass(), and
// importMaterials() methods are based on source code from Nate Robins' OpenGL
// Tutors programs (http://www.xmission.com/~nate/tutors.html).
//
//-----------------------------------------------------------------------------

#include "model_obj.hpp"
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <GLES2/gl2platform.h>

namespace
{
    bool MeshCompFunc(const ModelOBJ::Mesh &lhs, const ModelOBJ::Mesh &rhs)
    {
        return lhs.pMaterial->alpha > rhs.pMaterial->alpha;
    }
}

ModelOBJ::ModelOBJ()
{
    m_hasPositions = false;
    m_hasNormals = false;
    m_hasTextureCoords = false;
    m_hasTangents = false;

    m_numberOfVertexCoords = 0;
    m_numberOfTextureCoords = 0;
    m_numberOfNormals = 0;
    m_numberOfTriangles = 0;
    m_numberOfMaterials = 0;
    m_numberOfMeshes = 0;

    m_center[0] = m_center[1] = m_center[2] = 0.0f;
    m_width = m_height = m_length = m_radius = 0.0f;
}

ModelOBJ::~ModelOBJ()
{
    destroy();
}

void ModelOBJ::bounds(float center[3], float &width, float &height,
                      float &length, float &radius) const
{
    float xMax = 0;
    float yMax = 0;
    float zMax = 0;

    float xMin = 0;
    float yMin = 0;
    float zMin = 0;

    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;

    int numVerts = static_cast<int>(m_vertexBuffer.size());

	if(numVerts > 0)
	{
		 xMax = m_vertexBuffer[0].position[0];
		 yMax = m_vertexBuffer[0].position[1];
		 zMax = m_vertexBuffer[0].position[2];

		 xMin = m_vertexBuffer[0].position[0];
		 yMin = m_vertexBuffer[0].position[1];
		 zMin = m_vertexBuffer[0].position[2];
	}

	double xsum = 0, ysum = 0, zsum = 0;

    for (int i = 0; i < numVerts; ++i)
    {
        x = m_vertexBuffer[i].position[0];
        y = m_vertexBuffer[i].position[1];
        z = m_vertexBuffer[i].position[2];

		xsum += x;
		ysum += y;
		zsum += z;

        if (x < xMin)
            xMin = x;
        if (x > xMax)
            xMax = x;
        if (y < yMin)
            yMin = y;
        if (y > yMax)
            yMax = y;
        if (z < zMin)
            zMin = z;
        if (z > zMax)
            zMax = z;
    }

   /* center[0] = (xMin + xMax) / 2.0f;
    center[1] = (yMin + yMax) / 2.0f;
    center[2] = (zMin + zMax) / 2.0f;*/

	center[0] = xsum / numVerts;
    center[1] = ysum / numVerts;
    center[2] = zsum / numVerts;

    width = xMax - xMin;
    height = yMax - yMin;
    length = zMax - zMin;

    radius = std::max(std::max(width, height), length);
}

void ModelOBJ::destroy()
{
    m_hasPositions = false;
    m_hasTextureCoords = false;
    m_hasNormals = false;
    m_hasTangents = false;

    m_numberOfVertexCoords = 0;
    m_numberOfTextureCoords = 0;
    m_numberOfNormals = 0;
    m_numberOfTriangles = 0;
    m_numberOfMaterials = 0;
    m_numberOfMeshes = 0;

    m_center[0] = m_center[1] = m_center[2] = 0.0f;
    m_width = m_height = m_length = m_radius = 0.0f;

    m_directoryPath.clear();

    m_meshes.clear();
    m_materials.clear();
    m_vertexBuffer.clear();
    m_indexBuffer.clear();
    m_attributeBuffer.clear();

    m_vertexCoords.clear();
    m_textureCoords.clear();
    m_normals.clear();

    m_materialCache.clear();
    m_vertexCache.clear();
}

bool ModelOBJ::import(const std::string& filename, bool rebuildNormals)
{
    std::cout << "ModelOBJ loading: " << filename << std::endl;

    FILE *pFile = fopen(filename.c_str(), "r");

    if (!pFile)
        return false;

    // Extract the directory the OBJ file is in from the file name.
    // This directory path will be used to load the OBJ's associated MTL file.

    m_directoryPath.clear();

    //std::string filename = pszFilename;
    std::string::size_type offset = filename.find_last_of('\\');

    if (offset != std::string::npos)
    {
        m_directoryPath = filename.substr(0, ++offset);
    }
    else
    {
        offset = filename.find_last_of('/');

        if (offset != std::string::npos)
            m_directoryPath = filename.substr(0, ++offset);
    }

    // Import the OBJ file.

    importGeometryFirstPass(pFile);
    rewind(pFile);
    importGeometrySecondPass(pFile);
    fclose(pFile);

    // Perform post import tasks.

    buildMeshes();
    bounds(m_center, m_width, m_height, m_length, m_radius);

    // Build vertex normals if required.

    if (rebuildNormals)
    {
        generateNormals();
    }
    else
    {
        if (!hasNormals())
            generateNormals();
    }
	
    // Build tangents is required.

  /*  for (int i = 0; i < m_numberOfMaterials; ++i)
    {
        if (!m_materials[i].bumpMapFilename.empty())
        {
            generateTangents();
            break;
        }
    }*/

	//generateNormals();
	generateTangents();
    return true;
}

void ModelOBJ::normalize(float scaleTo, bool center)
{
    float width = 0.0f;
    float height = 0.0f;
    float length = 0.0f;
    float radius = 0.0f;
    float centerPos[3] = {0.0f};

    bounds(centerPos, width, height, length, radius);

    float scalingFactor = scaleTo / radius;
    float offset[3] = {0.0f};

    if (center)
    {
        offset[0] = -centerPos[0];
        offset[1] = -centerPos[1];
        offset[2] = -centerPos[2];
    }
    else
    {
        offset[0] = 0.0f;
        offset[1] = 0.0f;
        offset[2] = 0.0f;
    }

    scale(scalingFactor, offset);
    bounds(m_center, m_width, m_height, m_length, m_radius);
}



void ModelOBJ::reverseWinding()
{
    int swap = 0;

    // Reverse face winding.
    for (int i = 0; i < static_cast<int>(m_indexBuffer.size()); i += 3)
    {
        swap = m_indexBuffer[i + 1];
        m_indexBuffer[i + 1] = m_indexBuffer[i + 2];
        m_indexBuffer[i + 2] = swap;
    }

    float *pNormal = 0;
    float *pTangent = 0;

    // Invert normals and tangents.
    for (int i = 0; i < static_cast<int>(m_vertexBuffer.size()); ++i)
    {
        pNormal = m_vertexBuffer[i].normal;
        pNormal[0] = -pNormal[0];
        pNormal[1] = -pNormal[1];
        pNormal[2] = -pNormal[2];

        pTangent = m_vertexBuffer[i].tangent;
        pTangent[0] = -pTangent[0];
        pTangent[1] = -pTangent[1];
        pTangent[2] = -pTangent[2];
    }
}

void ModelOBJ::scale(float scaleFactor, float offset[3])
{
    float *pPosition = 0;

    for (int i = 0; i < static_cast<int>(m_vertexBuffer.size()); ++i)
    {
        pPosition = m_vertexBuffer[i].position;

        pPosition[0] += offset[0];
        pPosition[1] += offset[1];
        pPosition[2] += offset[2];

        pPosition[0] *= scaleFactor;
        pPosition[1] *= scaleFactor;
        pPosition[2] *= scaleFactor;
    }
}

void ModelOBJ::addTrianglePos(int index, int material, int v0, int v1, int v2)
{
    Vertex vertex =
    {
        0.0f, 0.0f, 0.0f,
        0.0f, 0.0f,
        0.0f, 0.0f, 0.0f
    };

    m_attributeBuffer[index] = material;

    vertex.position[0] = m_vertexCoords[v0 * 3];
    vertex.position[1] = m_vertexCoords[v0 * 3 + 1];
    vertex.position[2] = m_vertexCoords[v0 * 3 + 2];
    m_indexBuffer[index * 3] = addVertex(v0, &vertex);

    vertex.position[0] = m_vertexCoords[v1 * 3];
    vertex.position[1] = m_vertexCoords[v1 * 3 + 1];
    vertex.position[2] = m_vertexCoords[v1 * 3 + 2];
    m_indexBuffer[index * 3 + 1] = addVertex(v1, &vertex);

    vertex.position[0] = m_vertexCoords[v2 * 3];
    vertex.position[1] = m_vertexCoords[v2 * 3 + 1];
    vertex.position[2] = m_vertexCoords[v2 * 3 + 2];
    m_indexBuffer[index * 3 + 2] = addVertex(v2, &vertex);
}

void ModelOBJ::addTrianglePosNormal(int index, int material, int v0, int v1,
                                    int v2, int vn0, int vn1, int vn2)
{
    Vertex vertex =
    {
        0.0f, 0.0f, 0.0f,
        0.0f, 0.0f,
        0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f
    };

    m_attributeBuffer[index] = material;

    vertex.position[0] = m_vertexCoords[v0 * 3];
    vertex.position[1] = m_vertexCoords[v0 * 3 + 1];
    vertex.position[2] = m_vertexCoords[v0 * 3 + 2];
    vertex.normal[0] = m_normals[vn0 * 3];
    vertex.normal[1] = m_normals[vn0 * 3 + 1];
    vertex.normal[2] = m_normals[vn0 * 3 + 2];
    m_indexBuffer[index * 3] = addVertex(v0, &vertex);

    vertex.position[0] = m_vertexCoords[v1 * 3];
    vertex.position[1] = m_vertexCoords[v1 * 3 + 1];
    vertex.position[2] = m_vertexCoords[v1 * 3 + 2];
    vertex.normal[0] = m_normals[vn1 * 3];
    vertex.normal[1] = m_normals[vn1 * 3 + 1];
    vertex.normal[2] = m_normals[vn1 * 3 + 2];
    m_indexBuffer[index * 3 + 1] = addVertex(v1, &vertex);

    vertex.position[0] = m_vertexCoords[v2 * 3];
    vertex.position[1] = m_vertexCoords[v2 * 3 + 1];
    vertex.position[2] = m_vertexCoords[v2 * 3 + 2];
    vertex.normal[0] = m_normals[vn2 * 3];
    vertex.normal[1] = m_normals[vn2 * 3 + 1];
    vertex.normal[2] = m_normals[vn2 * 3 + 2];
    m_indexBuffer[index * 3 + 2] = addVertex(v2, &vertex);
}

void ModelOBJ::addTrianglePosTexCoord(int index, int material, int v0, int v1,
                                      int v2, int vt0, int vt1, int vt2)
{
    Vertex vertex =
    {
        0.0f, 0.0f, 0.0f,
        0.0f, 0.0f,
        0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f
    };

    m_attributeBuffer[index] = material;

    vertex.position[0] = m_vertexCoords[v0 * 3];
    vertex.position[1] = m_vertexCoords[v0 * 3 + 1];
    vertex.position[2] = m_vertexCoords[v0 * 3 + 2];
    vertex.texCoord[0] = m_textureCoords[vt0 * 2];
    vertex.texCoord[1] = m_textureCoords[vt0 * 2 + 1];
    m_indexBuffer[index * 3] = addVertex(v0, &vertex);

    vertex.position[0] = m_vertexCoords[v1 * 3];
    vertex.position[1] = m_vertexCoords[v1 * 3 + 1];
    vertex.position[2] = m_vertexCoords[v1 * 3 + 2];
    vertex.texCoord[0] = m_textureCoords[vt1 * 2];
    vertex.texCoord[1] = m_textureCoords[vt1 * 2 + 1];
    m_indexBuffer[index * 3 + 1] = addVertex(v1, &vertex);

    vertex.position[0] = m_vertexCoords[v2 * 3];
    vertex.position[1] = m_vertexCoords[v2 * 3 + 1];
    vertex.position[2] = m_vertexCoords[v2 * 3 + 2];
    vertex.texCoord[0] = m_textureCoords[vt2 * 2];
    vertex.texCoord[1] = m_textureCoords[vt2 * 2 + 1];
    m_indexBuffer[index * 3 + 2] = addVertex(v2, &vertex);
}

void ModelOBJ::addTrianglePosTexCoordNormal(int index, int material, int v0,
                                            int v1, int v2, int vt0, int vt1,
                                            int vt2, int vn0, int vn1, int vn2)
{
    Vertex vertex =
    {
        0.0f, 0.0f, 0.0f,
        0.0f, 0.0f,
        0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f
    };

    m_attributeBuffer[index] = material;

    vertex.position[0] = m_vertexCoords[v0 * 3];
    vertex.position[1] = m_vertexCoords[v0 * 3 + 1];
    vertex.position[2] = m_vertexCoords[v0 * 3 + 2];
    vertex.texCoord[0] = m_textureCoords[vt0 * 2];
    vertex.texCoord[1] = m_textureCoords[vt0 * 2 + 1];
    vertex.normal[0] = m_normals[vn0 * 3];
    vertex.normal[1] = m_normals[vn0 * 3 + 1];
    vertex.normal[2] = m_normals[vn0 * 3 + 2];
    m_indexBuffer[index * 3] = addVertex(v0, &vertex);

    vertex.position[0] = m_vertexCoords[v1 * 3];
    vertex.position[1] = m_vertexCoords[v1 * 3 + 1];
    vertex.position[2] = m_vertexCoords[v1 * 3 + 2];
    vertex.texCoord[0] = m_textureCoords[vt1 * 2];
    vertex.texCoord[1] = m_textureCoords[vt1 * 2 + 1];
    vertex.normal[0] = m_normals[vn1 * 3];
    vertex.normal[1] = m_normals[vn1 * 3 + 1];
    vertex.normal[2] = m_normals[vn1 * 3 + 2];
    m_indexBuffer[index * 3 + 1] = addVertex(v1, &vertex);

    vertex.position[0] = m_vertexCoords[v2 * 3];
    vertex.position[1] = m_vertexCoords[v2 * 3 + 1];
    vertex.position[2] = m_vertexCoords[v2 * 3 + 2];
    vertex.texCoord[0] = m_textureCoords[vt2 * 2];
    vertex.texCoord[1] = m_textureCoords[vt2 * 2 + 1];
    vertex.normal[0] = m_normals[vn2 * 3];
    vertex.normal[1] = m_normals[vn2 * 3 + 1];
    vertex.normal[2] = m_normals[vn2 * 3 + 2];
    m_indexBuffer[index * 3 + 2] = addVertex(v2, &vertex);
}

int ModelOBJ::addVertex(int hash, const Vertex *pVertex)
{
    int index = -1;
    std::map<int, std::vector<int> >::const_iterator iter = m_vertexCache.find(hash);

    if (iter == m_vertexCache.end())
    {
        // Vertex hash doesn't exist in the cache.

        index = static_cast<int>(m_vertexBuffer.size());
        m_vertexBuffer.push_back(*pVertex);
        m_vertexCache.insert(std::make_pair(hash, std::vector<int>(1, index)));
    }
    else
    {
        // One or more vertices have been hashed to this entry in the cache.

        const std::vector<int> &vertices = iter->second;
        const Vertex *pCachedVertex = 0;
        bool found = false;

        for (std::vector<int>::const_iterator i = vertices.begin(); i != vertices.end(); ++i)
        {
            index = *i;
            pCachedVertex = &m_vertexBuffer[index];

            if (memcmp(pCachedVertex, pVertex, sizeof(Vertex)) == 0)
            {
                found = true;
                break;
            }
        }

        if (!found)
        {
            index = static_cast<int>(m_vertexBuffer.size());
            m_vertexBuffer.push_back(*pVertex);
            m_vertexCache[hash].push_back(index);
        }
    }

    return index;
}

void ModelOBJ::buildMeshes()
{
    // Group the model's triangles based on material type.

    Mesh *pMesh = 0;
    int materialId = -1;
    int numMeshes = 0;

    // Count the number of meshes.
    for (int i = 0; i < static_cast<int>(m_attributeBuffer.size()); ++i)
    {
        if (m_attributeBuffer[i] != materialId)
        {
            materialId = m_attributeBuffer[i];
            ++numMeshes;
        }
    }

    // Allocate memory for the meshes and reset counters.
    m_numberOfMeshes = numMeshes;
    m_meshes.resize(m_numberOfMeshes);
    numMeshes = 0;
    materialId = -1;

    // Build the meshes. One mesh for each unique material.
    for (int i = 0; i < static_cast<int>(m_attributeBuffer.size()); ++i)
    {
        if (m_attributeBuffer[i] != materialId)
        {
            materialId = m_attributeBuffer[i];
            pMesh = &m_meshes[numMeshes++];            
            pMesh->pMaterial = &m_materials[materialId];
            pMesh->startIndex = i * 3;
            ++pMesh->triangleCount;
        }
        else
        {
            ++pMesh->triangleCount;
        }
    }

    // Sort the meshes based on its material alpha. Fully opaque meshes
    // towards the front and fully transparent towards the back.
    std::sort(m_meshes.begin(), m_meshes.end(), MeshCompFunc);
}

void ModelOBJ::generateNormals()
{
    const int *pTriangle = 0;
    Vertex *pVertex0 = 0;
    Vertex *pVertex1 = 0;
    Vertex *pVertex2 = 0;
    float edge1[3] = {0.0f, 0.0f, 0.0f};
    float edge2[3] = {0.0f, 0.0f, 0.0f};
    float normal[3] = {0.0f, 0.0f, 0.0f};
    float length = 0.0f;
    int totalVertices = getNumberOfVertices();
    int totalTriangles = getNumberOfTriangles();

    // Initialize all the vertex normals.
    for (int i = 0; i < totalVertices; ++i)
    {
        pVertex0 = &m_vertexBuffer[i];
        pVertex0->normal[0] = 0.0f;
        pVertex0->normal[1] = 0.0f;
        pVertex0->normal[2] = 0.0f;
    }

    // Calculate the vertex normals.
    for (int i = 0; i < totalTriangles; ++i)
    {
        pTriangle = &m_indexBuffer[i * 3];

        pVertex0 = &m_vertexBuffer[pTriangle[0]];
        pVertex1 = &m_vertexBuffer[pTriangle[1]];
        pVertex2 = &m_vertexBuffer[pTriangle[2]];

        // Calculate triangle face normal.

        edge1[0] = pVertex1->position[0] - pVertex0->position[0];
        edge1[1] = pVertex1->position[1] - pVertex0->position[1];
        edge1[2] = pVertex1->position[2] - pVertex0->position[2];

        edge2[0] = pVertex2->position[0] - pVertex0->position[0];
        edge2[1] = pVertex2->position[1] - pVertex0->position[1];
        edge2[2] = pVertex2->position[2] - pVertex0->position[2];

        normal[0] = (edge1[1] * edge2[2]) - (edge1[2] * edge2[1]);
        normal[1] = (edge1[2] * edge2[0]) - (edge1[0] * edge2[2]);
        normal[2] = (edge1[0] * edge2[1]) - (edge1[1] * edge2[0]);

        // Accumulate the normals.

        pVertex0->normal[0] += normal[0];
        pVertex0->normal[1] += normal[1];
        pVertex0->normal[2] += normal[2];

        pVertex1->normal[0] += normal[0];
        pVertex1->normal[1] += normal[1];
        pVertex1->normal[2] += normal[2];

        pVertex2->normal[0] += normal[0];
        pVertex2->normal[1] += normal[1];
        pVertex2->normal[2] += normal[2];
    }

    // Normalize the vertex normals.
    for (int i = 0; i < totalVertices; ++i)
    {
        pVertex0 = &m_vertexBuffer[i];

        length = 1.0f / sqrtf(pVertex0->normal[0] * pVertex0->normal[0] +
            pVertex0->normal[1] * pVertex0->normal[1] +
            pVertex0->normal[2] * pVertex0->normal[2]);

        pVertex0->normal[0] *= length;
        pVertex0->normal[1] *= length;
        pVertex0->normal[2] *= length;
    }

    m_hasNormals = true;
}

void ModelOBJ::generateTangents()
{
    const int *pTriangle = 0;
    Vertex *pVertex0 = 0;
    Vertex *pVertex1 = 0;
    Vertex *pVertex2 = 0;
    float edge1[3] = {0.0f, 0.0f, 0.0f};
    float edge2[3] = {0.0f, 0.0f, 0.0f};
    float texEdge1[2] = {0.0f, 0.0f};
    float texEdge2[2] = {0.0f, 0.0f};
    float tangent[3] = {0.0f, 0.0f, 0.0f};
    float bitangent[3] = {0.0f, 0.0f, 0.0f};
    float det = 0.0f;
    float nDotT = 0.0f;
    float bDotB = 0.0f;
    float length = 0.0f;
    int totalVertices = getNumberOfVertices();
    int totalTriangles = getNumberOfTriangles();

    // Initialize all the vertex tangents and bitangents.
    for (int i = 0; i < totalVertices; ++i)
    {
        pVertex0 = &m_vertexBuffer[i];

        pVertex0->tangent[0] = 0.0f;
        pVertex0->tangent[1] = 0.0f;
        pVertex0->tangent[2] = 0.0f;
        pVertex0->tangent[3] = 0.0f;

        pVertex0->bitangent[0] = 0.0f;
        pVertex0->bitangent[1] = 0.0f;
        pVertex0->bitangent[2] = 0.0f;
    }

    // Calculate the vertex tangents and bitangents.
    for (int i = 0; i < totalTriangles; ++i)
    {
        pTriangle = &m_indexBuffer[i * 3];

        pVertex0 = &m_vertexBuffer[pTriangle[0]];
        pVertex1 = &m_vertexBuffer[pTriangle[1]];
        pVertex2 = &m_vertexBuffer[pTriangle[2]];

        // Calculate the triangle face tangent and bitangent.

        edge1[0] = pVertex1->position[0] - pVertex0->position[0];
        edge1[1] = pVertex1->position[1] - pVertex0->position[1];
        edge1[2] = pVertex1->position[2] - pVertex0->position[2];

        edge2[0] = pVertex2->position[0] - pVertex0->position[0];
        edge2[1] = pVertex2->position[1] - pVertex0->position[1];
        edge2[2] = pVertex2->position[2] - pVertex0->position[2];

        texEdge1[0] = pVertex1->texCoord[0] - pVertex0->texCoord[0];
        texEdge1[1] = pVertex1->texCoord[1] - pVertex0->texCoord[1];

        texEdge2[0] = pVertex2->texCoord[0] - pVertex0->texCoord[0];
        texEdge2[1] = pVertex2->texCoord[1] - pVertex0->texCoord[1];

        det = texEdge1[0] * texEdge2[1] - texEdge2[0] * texEdge1[1];

        if (fabs(det) < 1e-6f)
        {
            tangent[0] = 1.0f;
            tangent[1] = 0.0f;
            tangent[2] = 0.0f;

            bitangent[0] = 0.0f;
            bitangent[1] = 1.0f;
            bitangent[2] = 0.0f;
        }
        else
        {
            det = 1.0f / det;

            tangent[0] = (texEdge2[1] * edge1[0] - texEdge1[1] * edge2[0]) * det;
            tangent[1] = (texEdge2[1] * edge1[1] - texEdge1[1] * edge2[1]) * det;
            tangent[2] = (texEdge2[1] * edge1[2] - texEdge1[1] * edge2[2]) * det;

            bitangent[0] = (-texEdge2[0] * edge1[0] + texEdge1[0] * edge2[0]) * det;
            bitangent[1] = (-texEdge2[0] * edge1[1] + texEdge1[0] * edge2[1]) * det;
            bitangent[2] = (-texEdge2[0] * edge1[2] + texEdge1[0] * edge2[2]) * det;
        }

        // Accumulate the tangents and bitangents.

        pVertex0->tangent[0] += tangent[0];
        pVertex0->tangent[1] += tangent[1];
        pVertex0->tangent[2] += tangent[2];
        pVertex0->bitangent[0] += bitangent[0];
        pVertex0->bitangent[1] += bitangent[1];
        pVertex0->bitangent[2] += bitangent[2];

        pVertex1->tangent[0] += tangent[0];
        pVertex1->tangent[1] += tangent[1];
        pVertex1->tangent[2] += tangent[2];
        pVertex1->bitangent[0] += bitangent[0];
        pVertex1->bitangent[1] += bitangent[1];
        pVertex1->bitangent[2] += bitangent[2];

        pVertex2->tangent[0] += tangent[0];
        pVertex2->tangent[1] += tangent[1];
        pVertex2->tangent[2] += tangent[2];
        pVertex2->bitangent[0] += bitangent[0];
        pVertex2->bitangent[1] += bitangent[1];
        pVertex2->bitangent[2] += bitangent[2];
    }
	
    // Orthogonalize and normalize the vertex tangents.
    for (int i = 0; i < totalVertices; ++i)
    {
        pVertex0 = &m_vertexBuffer[i];

        // Gram-Schmidt orthogonalize tangent with normal.

        nDotT = pVertex0->normal[0] * pVertex0->tangent[0] +
                pVertex0->normal[1] * pVertex0->tangent[1] +
                pVertex0->normal[2] * pVertex0->tangent[2];

        pVertex0->tangent[0] -= pVertex0->normal[0] * nDotT;
        pVertex0->tangent[1] -= pVertex0->normal[1] * nDotT;
        pVertex0->tangent[2] -= pVertex0->normal[2] * nDotT;
	
		
        // Normalize the tangent.

        length = 1.0f / sqrtf(pVertex0->tangent[0] * pVertex0->tangent[0] +
                              pVertex0->tangent[1] * pVertex0->tangent[1] +
                              pVertex0->tangent[2] * pVertex0->tangent[2]);

        pVertex0->tangent[0] *= length;
        pVertex0->tangent[1] *= length;
        pVertex0->tangent[2] *= length;

        // Calculate the handedness of the local tangent space.
        // The bitangent vector is the cross product between the triangle face
        // normal vector and the calculated tangent vector. The resulting
        // bitangent vector should be the same as the bitangent vector
        // calculated from the set of linear equations above. If they point in
        // different directions then we need to invert the cross product
        // calculated bitangent vector. We store this scalar multiplier in the
        // tangent vector's 'w' component so that the correct bitangent vector
        // can be generated in the normal mapping shader's vertex shader.
        //
        // Normal maps have a left handed coordinate system with the origin
        // located at the top left of the normal map texture. The x coordinates
        // run horizontally from left to right. The y coordinates run
        // vertically from top to bottom. The z coordinates run out of the
        // normal map texture towards the viewer. Our handedness calculations
        // must take this fact into account as well so that the normal mapping
        // shader's vertex shader will generate the correct bitangent vectors.
        // Some normal map authoring tools such as Crazybump
        // (http://www.crazybump.com/) includes options to allow you to control
        // the orientation of the normal map normal's y-axis.

        bitangent[0] = (pVertex0->normal[1] * pVertex0->tangent[2]) - 
                       (pVertex0->normal[2] * pVertex0->tangent[1]);
        bitangent[1] = (pVertex0->normal[2] * pVertex0->tangent[0]) -
                       (pVertex0->normal[0] * pVertex0->tangent[2]);
        bitangent[2] = (pVertex0->normal[0] * pVertex0->tangent[1]) - 
                       (pVertex0->normal[1] * pVertex0->tangent[0]);

        bDotB = bitangent[0] * pVertex0->bitangent[0] + 
                bitangent[1] * pVertex0->bitangent[1] + 
                bitangent[2] * pVertex0->bitangent[2];

        pVertex0->tangent[3] = (bDotB < 0.0f) ? 1.0f : -1.0f;

        pVertex0->bitangent[0] = bitangent[0] * pVertex0->tangent[3];
        pVertex0->bitangent[1] = bitangent[1] * pVertex0->tangent[3];
        pVertex0->bitangent[2] = bitangent[2] * pVertex0->tangent[3];
    }
	
    m_hasTangents = true;
}

void ModelOBJ::importGeometryFirstPass(FILE *pFile)
{
    m_hasTextureCoords = false;
    m_hasNormals = false;

    m_numberOfVertexCoords = 0;
    m_numberOfTextureCoords = 0;
    m_numberOfNormals = 0;
    m_numberOfTriangles = 0;

    int v = 0;
    int vt = 0;
    int vn = 0;
    char buffer[256] = {0};
    std::string name;

    while (fscanf(pFile, "%s", buffer) != EOF)
    {
        switch (buffer[0])
        {
        case 'f':   // v, v//vn, v/vt, v/vt/vn.
            fscanf(pFile, "%s", buffer);

            if (strstr(buffer, "//")) // v//vn
            {
                sscanf(buffer, "%d//%d", &v, &vn);
                fscanf(pFile, "%d//%d", &v, &vn);
                fscanf(pFile, "%d//%d", &v, &vn);
                ++m_numberOfTriangles;

                while (fscanf(pFile, "%d//%d", &v, &vn) > 0)
                    ++m_numberOfTriangles;
            }
            else if (sscanf(buffer, "%d/%d/%d", &v, &vt, &vn) == 3) // v/vt/vn
            {
                fscanf(pFile, "%d/%d/%d", &v, &vt, &vn);
                fscanf(pFile, "%d/%d/%d", &v, &vt, &vn);
                ++m_numberOfTriangles;

                while (fscanf(pFile, "%d/%d/%d", &v, &vt, &vn) > 0)
                    ++m_numberOfTriangles;
            }
            else if (sscanf(buffer, "%d/%d", &v, &vt) == 2) // v/vt
            {
                fscanf(pFile, "%d/%d", &v, &vt);
                fscanf(pFile, "%d/%d", &v, &vt);
                ++m_numberOfTriangles;

                while (fscanf(pFile, "%d/%d", &v, &vt) > 0)
                    ++m_numberOfTriangles;
            }
            else // v
            {
                fscanf(pFile, "%d", &v);
                fscanf(pFile, "%d", &v);
                ++m_numberOfTriangles;

                while (fscanf(pFile, "%d", &v) > 0)
                    ++m_numberOfTriangles;
            }
            break;

        case 'm':   // mtllib
            fgets(buffer, sizeof(buffer), pFile);
            sscanf(buffer, "%s %s", buffer, buffer);
            name = m_directoryPath;
            name += buffer;
            importMaterials(name.c_str());
            break;

        case 'v':   // v, vt, or vn
            switch (buffer[1])
            {
            case '\0':
                fgets(buffer, sizeof(buffer), pFile);
                ++m_numberOfVertexCoords;
                break;

            case 'n':
                fgets(buffer, sizeof(buffer), pFile);
                ++m_numberOfNormals;
                break;

            case 't':
                fgets(buffer, sizeof(buffer), pFile);
                ++m_numberOfTextureCoords;

            default:
                break;
            }
            break;

        default:
            fgets(buffer, sizeof(buffer), pFile);
            break;
        }
    }

    m_hasPositions = m_numberOfVertexCoords > 0;
    m_hasNormals = m_numberOfNormals > 0;
    m_hasTextureCoords = m_numberOfTextureCoords > 0;

    // Allocate memory for the OBJ model data.
    m_vertexCoords.resize(m_numberOfVertexCoords * 3);
    m_textureCoords.resize(m_numberOfTextureCoords * 2);
    m_normals.resize(m_numberOfNormals * 3);
    m_indexBuffer.resize(m_numberOfTriangles * 3);
    m_attributeBuffer.resize(m_numberOfTriangles);

    // Define a default material if no materials were loaded.
    if (m_numberOfMaterials == 0)
    {
        Material defaultMaterial =
        {
            0.2f, 0.2f, 0.2f, 1.0f,
            0.8f, 0.8f, 0.8f, 1.0f,
            0.0f, 0.0f, 0.0f, 1.0f,
            0.0f,
            1.0f,
            std::string("default"),
            std::string(),
            std::string()
        };

        m_materials.push_back(defaultMaterial);
        m_materialCache[defaultMaterial.name] = 0;
    }
}

void ModelOBJ::importGeometrySecondPass(FILE *pFile)
{
    int v[3] = {0};
    int vt[3] = {0};
    int vn[3] = {0};
    int numVertices = 0;
    int numTexCoords = 0;
    int numNormals = 0;
    int numTriangles = 0;
    int activeMaterial = 0;
    char buffer[256] = {0};
    std::string name;
    std::map<std::string, int>::const_iterator iter;

    while (fscanf(pFile, "%s", buffer) != EOF)
    {
        switch (buffer[0])
        {
        case 'f': // v, v//vn, v/vt, or v/vt/vn.
            v[0]  = v[1]  = v[2]  = 0;
            vt[0] = vt[1] = vt[2] = 0;
            vn[0] = vn[1] = vn[2] = 0;

            fscanf(pFile, "%s", buffer);

            if (strstr(buffer, "//")) // v//vn
            {
                sscanf(buffer, "%d//%d", &v[0], &vn[0]);
                fscanf(pFile, "%d//%d", &v[1], &vn[1]);
                fscanf(pFile, "%d//%d", &v[2], &vn[2]);

                v[0] = (v[0] < 0) ? v[0] + numVertices - 1 : v[0] - 1;
                v[1] = (v[1] < 0) ? v[1] + numVertices - 1 : v[1] - 1;
                v[2] = (v[2] < 0) ? v[2] + numVertices - 1 : v[2] - 1;

                vn[0] = (vn[0] < 0) ? vn[0] + numNormals - 1 : vn[0] - 1;
                vn[1] = (vn[1] < 0) ? vn[1] + numNormals - 1 : vn[1] - 1;
                vn[2] = (vn[2] < 0) ? vn[2] + numNormals - 1 : vn[2] - 1;

                addTrianglePosNormal(numTriangles++, activeMaterial,
                    v[0], v[1], v[2], vn[0], vn[1], vn[2]);

                v[1] = v[2];
                vn[1] = vn[2];

                while (fscanf(pFile, "%d//%d", &v[2], &vn[2]) > 0)
                {
                    v[2] = (v[2] < 0) ? v[2] + numVertices - 1 : v[2] - 1;
                    vn[2] = (vn[2] < 0) ? vn[2] + numNormals - 1 : vn[2] - 1;

                    addTrianglePosNormal(numTriangles++, activeMaterial,
                        v[0], v[1], v[2], vn[0], vn[1], vn[2]);

                    v[1] = v[2];
                    vn[1] = vn[2];
                }
            }
            else if (sscanf(buffer, "%d/%d/%d", &v[0], &vt[0], &vn[0]) == 3) // v/vt/vn
            {
                fscanf(pFile, "%d/%d/%d", &v[1], &vt[1], &vn[1]);
                fscanf(pFile, "%d/%d/%d", &v[2], &vt[2], &vn[2]);

                v[0] = (v[0] < 0) ? v[0] + numVertices - 1 : v[0] - 1;
                v[1] = (v[1] < 0) ? v[1] + numVertices - 1 : v[1] - 1;
                v[2] = (v[2] < 0) ? v[2] + numVertices - 1 : v[2] - 1;

                vt[0] = (vt[0] < 0) ? vt[0] + numTexCoords - 1 : vt[0] - 1;
                vt[1] = (vt[1] < 0) ? vt[1] + numTexCoords - 1 : vt[1] - 1;
                vt[2] = (vt[2] < 0) ? vt[2] + numTexCoords - 1 : vt[2] - 1;

                vn[0] = (vn[0] < 0) ? vn[0] + numNormals - 1 : vn[0] - 1;
                vn[1] = (vn[1] < 0) ? vn[1] + numNormals - 1 : vn[1] - 1;
                vn[2] = (vn[2] < 0) ? vn[2] + numNormals - 1 : vn[2] - 1;

                addTrianglePosTexCoordNormal(numTriangles++, activeMaterial,
                    v[0], v[1], v[2], vt[0], vt[1], vt[2], vn[0], vn[1], vn[2]);

                v[1] = v[2];
                vt[1] = vt[2];
                vn[1] = vn[2];

                while (fscanf(pFile, "%d/%d/%d", &v[2], &vt[2], &vn[2]) > 0)
                {
                    v[2] = (v[2] < 0) ? v[2] + numVertices - 1 : v[2] - 1;
                    vt[2] = (vt[2] < 0) ? vt[2] + numTexCoords - 1 : vt[2] - 1;
                    vn[2] = (vn[2] < 0) ? vn[2] + numNormals - 1 : vn[2] - 1;

                    addTrianglePosTexCoordNormal(numTriangles++, activeMaterial,
                        v[0], v[1], v[2], vt[0], vt[1], vt[2], vn[0], vn[1], vn[2]);

                    v[1] = v[2];
                    vt[1] = vt[2];
                    vn[1] = vn[2];
                }
            }
            else if (sscanf(buffer, "%d/%d", &v[0], &vt[0]) == 2) // v/vt
            {
                fscanf(pFile, "%d/%d", &v[1], &vt[1]);
                fscanf(pFile, "%d/%d", &v[2], &vt[2]);

                v[0] = (v[0] < 0) ? v[0] + numVertices - 1 : v[0] - 1;
                v[1] = (v[1] < 0) ? v[1] + numVertices - 1 : v[1] - 1;
                v[2] = (v[2] < 0) ? v[2] + numVertices - 1 : v[2] - 1;

                vt[0] = (vt[0] < 0) ? vt[0] + numTexCoords - 1 : vt[0] - 1;
                vt[1] = (vt[1] < 0) ? vt[1] + numTexCoords - 1 : vt[1] - 1;
                vt[2] = (vt[2] < 0) ? vt[2] + numTexCoords - 1 : vt[2] - 1;

                addTrianglePosTexCoord(numTriangles++, activeMaterial,
                    v[0], v[1], v[2], vt[0], vt[1], vt[2]);

                v[1] = v[2];
                vt[1] = vt[2];

                while (fscanf(pFile, "%d/%d", &v[2], &vt[2]) > 0)
                {
                    v[2] = (v[2] < 0) ? v[2] + numVertices - 1 : v[2] - 1;
                    vt[2] = (vt[2] < 0) ? vt[2] + numTexCoords - 1 : vt[2] - 1;

                    addTrianglePosTexCoord(numTriangles++, activeMaterial,
                        v[0], v[1], v[2], vt[0], vt[1], vt[2]);

                    v[1] = v[2];
                    vt[1] = vt[2];
                }
            }
            else // v
            {
                sscanf(buffer, "%d", &v[0]);
                fscanf(pFile, "%d", &v[1]);
                fscanf(pFile, "%d", &v[2]);

                v[0] = (v[0] < 0) ? v[0] + numVertices - 1 : v[0] - 1;
                v[1] = (v[1] < 0) ? v[1] + numVertices - 1 : v[1] - 1;
                v[2] = (v[2] < 0) ? v[2] + numVertices - 1 : v[2] - 1;

                addTrianglePos(numTriangles++, activeMaterial, v[0], v[1], v[2]);

                v[1] = v[2];

                while (fscanf(pFile, "%d", &v[2]) > 0)
                {
                    v[2] = (v[2] < 0) ? v[2] + numVertices - 1 : v[2] - 1;

                    addTrianglePos(numTriangles++, activeMaterial, v[0], v[1], v[2]);

                    v[1] = v[2];
                }
            }
            break;

        case 'u': // usemtl
            fgets(buffer, sizeof(buffer), pFile);
            sscanf(buffer, "%s %s", buffer, buffer);
            name = buffer;
            iter = m_materialCache.find(buffer);
            activeMaterial = (iter == m_materialCache.end()) ? 0 : iter->second;
            break;

        case 'v': // v, vn, or vt.
            switch (buffer[1])
            {
            case '\0': // v
                fscanf(pFile, "%f %f %f",
                    &m_vertexCoords[3 * numVertices],
                    &m_vertexCoords[3 * numVertices + 1],
                    &m_vertexCoords[3 * numVertices + 2]);
                ++numVertices;
                break;

            case 'n': // vn
                fscanf(pFile, "%f %f %f",
                    &m_normals[3 * numNormals],
                    &m_normals[3 * numNormals + 1],
                    &m_normals[3 * numNormals + 2]);
                ++numNormals;
                break;

            case 't': // vt
                fscanf(pFile, "%f %f",
                    &m_textureCoords[2 * numTexCoords],
                    &m_textureCoords[2 * numTexCoords + 1]);
                ++numTexCoords;
                break;

            default:
                break;
            }
            break;

        default:
            fgets(buffer, sizeof(buffer), pFile);
            break;
        }
    }
}

bool ModelOBJ::importMaterials(const char *pszFilename)
{
    FILE *pFile = fopen(pszFilename, "r");

    if (!pFile)
        return false;

    Material *pMaterial = 0;
    int illum = 0;
    int numMaterials = 0;
    char buffer[256] = {0};

    // Count the number of materials in the MTL file.
    while (fscanf(pFile, "%s", buffer) != EOF)
    {
        switch (buffer[0])
        {
        case 'n': // newmtl
            ++numMaterials;
            fgets(buffer, sizeof(buffer), pFile);
            sscanf(buffer, "%s %s", buffer, buffer);
            break;

        default:
            fgets(buffer, sizeof(buffer), pFile);
            break;
        }
    }

    rewind(pFile);

    m_numberOfMaterials = numMaterials;
    numMaterials = 0;
    m_materials.resize(m_numberOfMaterials);

    // Load the materials in the MTL file.
    while (fscanf(pFile, "%s", buffer) != EOF)
    {
        switch (buffer[0])
        {
        case 'N': // Ns
            fscanf(pFile, "%f", &pMaterial->shininess);

            // Wavefront .MTL file shininess is from [0,1000].
            // Scale back to a generic [0,1] range.
            pMaterial->shininess /= 1000.0f;
            break;

        case 'K': // Ka, Kd, or Ks
            switch (buffer[1])
            {
            case 'a': // Ka
                fscanf(pFile, "%f %f %f",
                    &pMaterial->ambient[0],
                    &pMaterial->ambient[1],
                    &pMaterial->ambient[2]);
                pMaterial->ambient[3] = 1.0f;
                break;

            case 'd': // Kd
                fscanf(pFile, "%f %f %f",
                    &pMaterial->diffuse[0],
                    &pMaterial->diffuse[1],
                    &pMaterial->diffuse[2]);
                pMaterial->diffuse[3] = 1.0f;
                break;

            case 's': // Ks
                fscanf(pFile, "%f %f %f",
                    &pMaterial->specular[0],
                    &pMaterial->specular[1],
                    &pMaterial->specular[2]);
                pMaterial->specular[3] = 1.0f;
                break;

            default:
                fgets(buffer, sizeof(buffer), pFile);
                break;
            }
            break;

        case 'T': // Tr
            switch (buffer[1])
            {
            case 'r': // Tr
                fscanf(pFile, "%f", &pMaterial->alpha);
                pMaterial->alpha = 1.0f - pMaterial->alpha;
                break;

            default:
                fgets(buffer, sizeof(buffer), pFile);
                break;
            }
            break;

        case 'd':
            fscanf(pFile, "%f", &pMaterial->alpha);
            break;

        case 'i': // illum
            fscanf(pFile, "%d", &illum);

            if (illum == 1)
            {
                pMaterial->specular[0] = 0.0f;
                pMaterial->specular[1] = 0.0f;
                pMaterial->specular[2] = 0.0f;
                pMaterial->specular[3] = 1.0f;
            }
            break;

        case 'm': // map_Kd, map_bump
            if (strstr(buffer, "map_Kd") != 0)
            {
                fgets(buffer, sizeof(buffer), pFile);
                sscanf(buffer, "%s %s", buffer, buffer);
                pMaterial->colorMapFilename = buffer;
            }
            else if (strstr(buffer, "map_bump") != 0)
            {
                fgets(buffer, sizeof(buffer), pFile);
                sscanf(buffer, "%s %s", buffer, buffer);
                pMaterial->bumpMapFilename = buffer;
            }
            else
            {
                fgets(buffer, sizeof(buffer), pFile);
            }
            break;

        case 'n': // newmtl
            fgets(buffer, sizeof(buffer), pFile);
            sscanf(buffer, "%s %s", buffer, buffer);

            pMaterial = &m_materials[numMaterials];
            pMaterial->ambient[0] = 0.2f;
            pMaterial->ambient[1] = 0.2f;
            pMaterial->ambient[2] = 0.2f;
            pMaterial->ambient[3] = 1.0f;
            pMaterial->diffuse[0] = 0.8f;
            pMaterial->diffuse[1] = 0.8f;
            pMaterial->diffuse[2] = 0.8f;
            pMaterial->diffuse[3] = 1.0f;
            pMaterial->specular[0] = 0.0f;
            pMaterial->specular[1] = 0.0f;
            pMaterial->specular[2] = 0.0f;
            pMaterial->specular[3] = 1.0f;
            pMaterial->shininess = 0.0f;
            pMaterial->alpha = 1.0f;
            pMaterial->name = buffer;
            pMaterial->colorMapFilename.clear();
            pMaterial->bumpMapFilename.clear();

            m_materialCache[pMaterial->name] = numMaterials;
            ++numMaterials;
            break;

        default:
            fgets(buffer, sizeof(buffer), pFile);
            break;
        }
    }

    fclose(pFile);
    return true;
}

void ModelOBJ::buildShortIndexBuffer()
{
    m_shortIndexBuffer.clear();
    m_shortIndexBuffer.reserve(getNumberOfIndices());
    for(auto index : m_indexBuffer)
        m_shortIndexBuffer.push_back(static_cast<uint16_t>(index));
}

void ModelOBJ::buildVBOs()
{
    buildShortIndexBuffer();

	GLuint vn = getNumberOfVertices();

    positions.clear(); positions.reserve(vn);
    textureCoords.clear(); textureCoords.reserve(vn);
    normals.clear(); normals.reserve(vn);
    tangents.clear(); tangents.reserve(vn);
    bitangents.clear(); bitangents.reserve(vn);

    for(uint32_t i = 0; i < vn; ++i)
    {
        const Vertex& v = m_vertexBuffer[i];

        positions.push_back     (ruis::vec4 {v.position[0], v.position[1], v.position[2], 1.0f});
        textureCoords.push_back (ruis::vec2 {v.texCoord[0], v.texCoord[1]});
        normals.push_back       (ruis::vec3 {v.normal[0], v.normal[1], v.normal[2]});
        tangents.push_back      (ruis::vec4 {v.tangent[0], v.tangent[1], v.tangent[2], v.tangent[3]});
        bitangents.push_back    (ruis::vec4 {v.bitangent[0], v.bitangent[1], v.bitangent[2], 1.0f});
    }
}

void ModelOBJ::createVBOs()
{
    buildShortIndexBuffer(); // TODO: make it a non-kolkhoz fashion. OpenGLES 2 requires us to use 16-bit indices
    

    //glGenVertexArrays( 1, &vaoHandle );
    //glBindVertexArray(vaoHandle);

    glGenBuffers(2, vboHandles);

	GLuint vs = getVertexSize();
	GLuint vn = getNumberOfVertices();

    // kolkhoz #2:
    for(uint32_t i = 0; i < vn; ++i)
    {
        m_vertexBuffer[i].position[3] = 1.0f;
    }

    std::cout << "verts: " << vn << std::endl;

	GLubyte p_off = offsetof(struct Vertex, position);
    GLubyte tc_off = offsetof(struct Vertex, texCoord);
	GLubyte n_off = offsetof(struct Vertex, normal);
	GLubyte t_off = offsetof(struct Vertex, tangent);
	GLubyte bt_off = offsetof(struct Vertex, bitangent);
	
    glBindBuffer(GL_ARRAY_BUFFER, vboHandles[0]);
    glBufferData(GL_ARRAY_BUFFER, vs * vn, getVertexBuffer(), GL_STATIC_DRAW);

	glVertexAttribPointer( (GLuint)0, 4, GL_FLOAT, GL_FALSE, vs,  ((GLubyte *)NULL + (p_off)));
    glEnableVertexAttribArray(0);  // Vertex position
    glVertexAttribPointer( (GLuint)1, 2, GL_FLOAT, GL_FALSE, vs,  ((GLubyte *)NULL + (tc_off)));
    glEnableVertexAttribArray(1);  // texture coords
    glVertexAttribPointer( (GLuint)2, 3, GL_FLOAT, GL_FALSE, vs,  ((GLubyte *)NULL + (n_off)));
    glEnableVertexAttribArray(2);  // Vertex normal	
	glVertexAttribPointer( (GLuint)3, 4, GL_FLOAT, GL_FALSE, vs,  ((GLubyte *)NULL + (t_off)));
	glEnableVertexAttribArray(3);  // Tangent vector
	glVertexAttribPointer( (GLuint)4, 3, GL_FLOAT, GL_FALSE, vs,  ((GLubyte *)NULL + (bt_off)));
	glEnableVertexAttribArray(4);  // beeTangent vector


    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboHandles[1]);
    //glBufferData(GL_ELEMENT_ARRAY_BUFFER, getNumberOfIndices()  * sizeof(GLuint), getIndexBuffer(), GL_STATIC_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, getNumberOfIndices() * sizeof(GLushort), getShortIndexBufferPtr(), GL_STATIC_DRAW);


    //glBindVertexArray(0);
}

void ModelOBJ::render() 
{
    GLuint vs = getVertexSize();
	GLuint vn = getNumberOfVertices();

    GLubyte p_off = offsetof(struct Vertex, position);
    GLubyte tc_off = offsetof(struct Vertex, texCoord);
	GLubyte n_off = offsetof(struct Vertex, normal);
	GLubyte t_off = offsetof(struct Vertex, tangent);
	GLubyte bt_off = offsetof(struct Vertex, bitangent);

//  glBindVertexArray(vaoHandle;
//	glDrawElements(GL_TRIANGLES, getNumberOfIndices(), GL_UNSIGNED_INT, ((GLubyte *)NULL + (0)));

    glBindBuffer(GL_ARRAY_BUFFER, vboHandles[0]);

    glVertexAttribPointer( (GLuint)0, 4, GL_FLOAT, GL_FALSE, vs,  ((GLubyte *)NULL + (p_off)));
    glEnableVertexAttribArray(0);  // Vertex position
    glVertexAttribPointer( (GLuint)1, 2, GL_FLOAT, GL_FALSE, vs,  ((GLubyte *)NULL + (tc_off)));
    glEnableVertexAttribArray(1);  // texture coords
    glVertexAttribPointer( (GLuint)2, 3, GL_FLOAT, GL_FALSE, vs,  ((GLubyte *)NULL + (n_off)));
    glEnableVertexAttribArray(2);  // Vertex normal	
	glVertexAttribPointer( (GLuint)3, 4, GL_FLOAT, GL_FALSE, vs,  ((GLubyte *)NULL + (t_off)));
	glEnableVertexAttribArray(3);  // Tangent vector
	glVertexAttribPointer( (GLuint)4, 3, GL_FLOAT, GL_FALSE, vs,  ((GLubyte *)NULL + (bt_off)));
	glEnableVertexAttribArray(4);  // beeTangent vector

    // glEnableVertexAttribArray(posLoc);
    // glVertexAttribPointer(posLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
    // glBindBuffer(GL_ARRAY_BUFFER, colorBuffer);
    // glEnableVertexAttribArray(colLoc);
    // glVertexAttribPointer(colLoc, 4, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboHandles[1]);
    glDrawElements(GL_TRIANGLES, vn, GL_UNSIGNED_SHORT, 0);
}


// void ModelOBJ::createVAO()
// {

//     glGenVertexArrays( 1, &vaoHandle );
//     glBindVertexArray(vaoHandle);

//     unsigned int handle[2];
//     glGenBuffers(2, handle);

// 	GLuint vs = getVertexSize();
// 	GLuint vn = getNumberOfVertices();

// 	GLubyte v_off = offsetof(struct Vertex, position);
// 	GLubyte n_off = offsetof(struct Vertex, normal);
// 	GLubyte tc_off = offsetof(struct Vertex, texCoord);
// 	GLubyte t_off = offsetof(struct Vertex, tangent);
// 	GLubyte bt_off = offsetof(struct Vertex, bitangent);
	

		
//     glBindBuffer(GL_ARRAY_BUFFER, handle[0]);
//     glBufferData(GL_ARRAY_BUFFER, vs * vn, getVertexBuffer(), GL_STATIC_DRAW);

// 	glVertexAttribPointer( (GLuint)0, 3, GL_FLOAT, GL_FALSE, vs,  ((GLubyte *)NULL + (v_off)));
//     glEnableVertexAttribArray(0);  // Vertex position
//     glVertexAttribPointer( (GLuint)1, 3, GL_FLOAT, GL_FALSE, vs,  ((GLubyte *)NULL + (n_off)));
//     glEnableVertexAttribArray(1);  // Vertex normal	
//     glVertexAttribPointer( (GLuint)2, 2, GL_FLOAT, GL_FALSE, vs,  ((GLubyte *)NULL + (tc_off)));
//     glEnableVertexAttribArray(2);  // texture coords
// 	glVertexAttribPointer( (GLuint)3, 4, GL_FLOAT, GL_FALSE, vs,  ((GLubyte *)NULL + (t_off)));
// 	glEnableVertexAttribArray(3);  // Tangent vector
// 	glVertexAttribPointer( (GLuint)4, 3, GL_FLOAT, GL_FALSE, vs,  ((GLubyte *)NULL + (bt_off)));
// 	glEnableVertexAttribArray(4);  // beeTangent vector

//     glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, handle[1]);
//     glBufferData(GL_ELEMENT_ARRAY_BUFFER, getNumberOfIndices()  * sizeof(GLuint), getIndexBuffer(), GL_STATIC_DRAW);
// 	int i = getNumberOfIndices() ;

//     glBindVertexArray(0);
// }

// void ModelOBJ::render() 
// {
//     glBindVertexArray(vaoHandle);
// 	glDrawElements(GL_TRIANGLES, getNumberOfIndices(), GL_UNSIGNED_INT, ((GLubyte *)NULL + (0)));
// }


/*

            glEnableClientState(GL_VERTEX_ARRAY);
            glVertexPointer(3, GL_FLOAT, g_model.getVertexSize(),
                g_model.getVertexBuffer()->position);
       
            glClientActiveTexture(GL_TEXTURE0);
            glEnableClientState(GL_TEXTURE_COORD_ARRAY);
            glTexCoordPointer(2, GL_FLOAT, g_model.getVertexSize(),
                g_model.getVertexBuffer()->texCoord);
    

            glEnableClientState(GL_NORMAL_ARRAY);
            glNormalPointer(GL_FLOAT, g_model.getVertexSize(),
                g_model.getVertexBuffer()->normal);
   
            glClientActiveTexture(GL_TEXTURE1);
            glEnableClientState(GL_TEXTURE_COORD_ARRAY);
            glTexCoordPointer(4, GL_FLOAT, g_model.getVertexSize(),
     
        glDrawElements(GL_TRIANGLES, pMesh->triangleCount * 3, GL_UNSIGNED_INT,
            g_model.getIndexBuffer() + pMesh->startIndex);


			*/