#ifndef _OBJLOADER_H_
#define _OBJLOADER_H_

#include <vector>
#include <mat.h>

using namespace std;

typedef vec3 tVertex;
typedef vec2 tTexCoord;

struct tMaterial
{
	char name[256];
	int illum;
	char map_Kd[256];
	float Ns;
	float Ni;
	float d;
	float Ka[3];
	float Kd[3];
	float Ks[3];
	float Tf[3];
	unsigned int texture;

	tMaterial() {
		memset(this, 0, sizeof(tMaterial));
	}
};

// support up to pentagon
struct tFace
{
	int n;
	int v[5];	//vertices
	int vt[5];	//text coords
	int vn[5];	//normals

	tFace() {
		memset(this, 0, sizeof(tFace));
	}
};

struct tPart
{
	char name[256];
	vector<tFace> faces;

	tPart() {
		name[0] = 0;
	}
};

class CObjLoader
{
public:
	CObjLoader() {
		materials.reserve(100000);

		vertexes.reserve(100000);
		texcoords.reserve(100000);
		normals.reserve(100000);

		parts.reserve(100000);

		_work_path[0] = 0;
		_loaded = false;
	}
	
	bool Load(const char* objfile, const char* mtlfile = NULL) {
		_splitpath(objfile, NULL, _work_path, NULL, NULL);

		if (mtlfile && *mtlfile) {
			loadMaterials(mtlfile);
		}
		if (objfile && *objfile) {
			loadObjects(objfile);
		}
		return true;
	}

	tVertex getVertex(int idx) { return vertexes.at(idx); }
	int getSize() { return vertexes.size(); }

	vector<tMaterial> materials;
	vector<tVertex> vertexes;
	vector<tTexCoord> texcoords;
	vector<tVertex> normals;
	vector<tPart> parts;

	bool loadObjects(const char* fileName) {
		FILE* fp = fopen(fileName, "r");
		if (!fp) return false;

		tPart part_;
		part_.name[0] = 0;
		parts.push_back(part_);

		tPart* part = (tPart*)&(*parts.rbegin());
		char buffer[1024];

		while (fscanf(fp, "%s", buffer) != EOF) {
			bool go_eol = true;

			if (!strncmp("#", buffer, 1)) {
			}
			else if (!strcmp("v", buffer)) {
				// Specifies a geometric vertex and its x y z coordinates.
				tVertex v;
				fscanf(fp, "%f %f %f", &v.x, &v.y, &v.z);
				vertexes.push_back(v);
			}
			else if (!strcmp("vn", buffer)) {
				// Specifies a normal vector with components i, j, and k. 
				tVertex v;
				fscanf(fp, "%f %f %f", &v.x, &v.y, &v.z);
				normals.push_back(v);
			}
			else if (!strcmp("vt", buffer)) {
				// Specifies a texture vertex and its u v coordinates.
				tTexCoord t;
				fscanf(fp, "%f %f", &t.x, &t.y);
				texcoords.push_back(t);
			}
			else if (!strcmp("f", buffer)) {
				// Using v, vt, and vn to represent geometric vertices, texture vertices,
				// and vertex normals, the statement would read:
				//
				//    f v/vt/vn v/vt/vn v/vt/vn v/vt/vn
				tFace f;
				fgets(buffer, 1024, fp);

				char* p = buffer;
				for (int i = 0; i < 5; ++i) {
					while (*p == ' ' || *p == '\t') p++;
					if (*p == '\0' || *p == '\r' || *p == '\n') break;

					f.v[i] = strtoul(p, &p, 10);
					if (*p && *p == '/') {
						p++;
						f.vt[i] = strtoul(p, &p, 10);
						if (*p && *p == '/') {
							p++;
							f.vn[i] = strtoul(p, &p, 10);
						}
					}
					f.n++;
				}

				if (part) part->faces.push_back(f);
				go_eol = false;
			}
			else if (!strcmp("usemtl", buffer)) {
				tPart part_;

				fscanf(fp, "%s", part_.name);
				parts.push_back(part_);

				part = (tPart*)&(*parts.rbegin());
			}
			else if (!strcmp("mtllib", buffer)) {
				fscanf(fp, "%s", buffer);

				char path[256];
				_makepath(path, NULL, _work_path, buffer, NULL);

				loadMaterials(path);
			}
			if (go_eol) fgets(buffer, 1024, fp);
		}
		fclose(fp);
		return true;
	}
	bool loadMaterials(const char* fileName) {
		FILE* fp = fopen(fileName, "r");
		if (!fp) return false;

		tMaterial* material = NULL;
		char buffer[1024];

		while (fscanf(fp, "%s", buffer) != EOF) {
			if (!strncmp("#", buffer, 1)) {
			}
			else if (!strcmp("newmtl", buffer)) {
				tMaterial material_;
				fscanf(fp, "%s", material_.name);

				materials.push_back(material_);
				material = (tMaterial*)&(*materials.rbegin());
			}
			else if (!strcmp("Ka", buffer)) {
				// defines the ambient color of the material to be (r,g,b)
				if (material) fscanf(fp, "%f %f %f", &material->Ka[0], &material->Ka[1], &material->Ka[2]);
			}
			else if (!strcmp("Kd", buffer)) {
				// defines the diffuse reflectivity color of the material to be (r,g,b)
				if (material) fscanf(fp, "%f %f %f", &material->Kd[0], &material->Kd[1], &material->Kd[2]);
			}
			else if (!strcmp("Ks", buffer)) {
				// defines the specular reflectivity color of the material to be (r,g,b)
				if (material) fscanf(fp, "%f %f %f", &material->Ks[0], &material->Ks[1], &material->Ks[2]);
			}
			else if (!strcmp("Tf", buffer)) {
				// specify the transmission filter of the current material to be (r,g,b)
				if (material) fscanf(fp, "%f %f %f", &material->Tf[0], &material->Tf[1], &material->Tf[2]);
			}
			else if (!strcmp("illum", buffer)) {
				// specifies the illumination model to use in the material
				//  "illum_#"can be a number from 0 to 10
				//	 0	Color on and Ambient off
				//	 1	Color on and Ambient on
				//	 2	Highlight on
				//	 3	Reflection on and Ray trace on
				//	 4	Transparency: Glass on Reflection: Ray trace on
				//	 5	Reflection: Fresnel on and Ray trace on
				//	 6	Transparency: Refraction on Reflection: Fresnel off and Ray trace on
				//	 7	Transparency: Refraction on Reflection: Fresnel on and Ray trace on
				//	 8	Reflection on and Ray trace off
				//	 9	Transparency: Glass on Reflection: Ray trace off
				//	 10	Casts shadows onto invisible surfaces
				if (material) fscanf(fp, "%i", &material->illum);
			}
			else if (!strcmp("map_Kd", buffer)) {
				if (material) fscanf(fp, "%s", material->map_Kd);
			}
			else if (!strcmp("Ns", buffer)) {
				if (material) fscanf(fp, "%f", &material->Ns);
			}
			else if (!strcmp("Ni", buffer)) {
				if (material) fscanf(fp, "%f", &material->Ni);
			}
			else if (!strcmp("d", buffer)) {
				if (material) fscanf(fp, "%f", &material->d);
			}
			fgets(buffer, 1024, fp);
		}
		fclose(fp);
		return true;
	}

private:
	char _work_path[_MAX_DIR];
	bool _loaded;
};

#endif