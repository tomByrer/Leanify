#include "xml.h"

#include <cstring>
#include <iostream>

#include "../leanify.h"
#include "../utils.h"



Xml::Xml(void *p, size_t s /*= 0*/) : Format(p, s), doc(true)
{
    unsigned char *q = (unsigned char *)p;

    const unsigned char utf8_bom[] = { 0xEF, 0xBB, 0xBF };

    // tinyxml2 does not support utf16
    /*
    const unsigned char utf16be_bom[] = { 0xFE, 0xFF };
    const unsigned char utf16le_bom[] = { 0xFF, 0xFE };
    */
    // skip utf8 bom
    if (!memcmp(q, utf8_bom, sizeof(utf8_bom)))
    {
        q += sizeof(utf8_bom);
    }
    /*      else if (!memcmp(q, utf16le_bom, sizeof(utf16le_bom)) || !memcmp(q, utf16be_bom, sizeof(utf16be_bom)))
    {
    q += sizeof(utf16le_bom);
    }
    */
    // skip spaces
    while (isspace(*q) && q < (unsigned char *)p + s)
    {
        q++;
    }
    // only parse the file if it starts with '<'
    if (*q == '<')
    {
        is_valid = (doc.Parse(fp, size) == 0);
    }
    else
    {
        is_valid = false;
    }
}




size_t Xml::Leanify(size_t size_leanified /*= 0*/)
{
    const char *root_name = doc.RootElement()->Name();
    // if the XML is fb2 file
    if (strcmp(root_name, "FictionBook") == 0)
    {
        if (is_verbose)
        {
            std::cout << "FB2 detected." << std::endl;
        }
        if (depth < max_depth)
        {
            depth++;

            // iterate through all binary element
            for (auto e = doc.RootElement()->FirstChildElement("binary"); e; e = e->NextSiblingElement("binary"))
            {
                for (int i = 1; i < depth; i++)
                {
                    std::cout << "-> ";
                }
                std::cout << e->Attribute("id") << std::endl;

                const char *base64_data = e->GetText();
                if (base64_data == nullptr)
                {
                    std::cout << "No data found." << std::endl;
                    continue;
                }
                size_t base64_len = strlen(base64_data);

                // 4 base64 character contains information of 3 bytes
                size_t binary_len = 3 * base64_len / 4;
                unsigned char *binary_data = new unsigned char[binary_len];

                if (Base64Decode(base64_data, base64_len, binary_data, &binary_len))
                {
                    std::cout << "Base64 decode error." << std::endl;
                    delete[] binary_data;
                    continue;
                }

                // Leanify embedded image
                binary_len = LeanifyFile(binary_data, binary_len);

                // allocate a few more bytes for padding
                size_t new_base64_len = 4 * binary_len / 3 + 4;
                char *new_base64_data = new char[new_base64_len];

                if (Base64Encode(binary_data, binary_len, new_base64_data, new_base64_len))
                {
                    std::cout << "Base64 encode error." << std::endl;
                }
                else if (strlen(new_base64_data) < base64_len)
                {
                    e->SetText(new_base64_data);
                }

                delete[] binary_data;
                delete[] new_base64_data;
            }
            depth--;
        }
    }
    else if (strcmp(root_name, "svg") == 0)
    {
        if (is_verbose)
        {
            std::cout << "SVG detected." << std::endl;
        }
        for (auto e = doc.RootElement()->FirstChildElement("metadata"); e; e = e->NextSiblingElement("metadata"))
        {
            doc.RootElement()->DeleteChild(e);
        }

        // remove empty attribute
        TraverseElements(doc.RootElement(), [](tinyxml2::XMLElement* e)
        {
            for (auto attr = e->FirstAttribute(); attr; attr = attr->Next())
            {
                auto value = attr->Value();
                if (value == nullptr || *value == 0)
                {
                    e->DeleteAttribute(attr->Name());
                }
            }
        });
    }

    // print leanified XML to memory
    tinyxml2::XMLPrinter printer(0, true);
    doc.Print(&printer);

    size_t new_size = printer.CStrSize() - 1;     // -1 for the \0
    fp -= size_leanified;
    if (new_size < size)
    {
        memcpy(fp, printer.CStr(), new_size);
        return new_size;
    }
    else if (size_leanified)
    {
        memmove(fp, fp + size_leanified, size);
    }
    return size;
}

void Xml::TraverseElements(tinyxml2::XMLElement *e, std::function<void(tinyxml2::XMLElement*)> callback)
{
    callback(e);

    for (e = e->FirstChildElement(); e; e = e->NextSiblingElement())
    {
        TraverseElements(e, callback);
    }
}


