#include "splash.h"

#include <algorithm>
#include <fstream>
#include <iostream>

#include <boost/json/src.hpp>
#include <cxxopts.hpp>

#define VERSION "v0.1.0"
#define DEBUG() (std::cout << __FUNCTION__ << std::endl)

Splash* Splash::m_instance;

Splash::Splash(std::string inputPath, std::string outputPath, bool debug) :
    m_astFilePath {inputPath},
    m_outputFilePath {outputPath},
    // create index w/ excludeDeclsFromPCH = 1, displayDiagnostics=1.
    m_index {clang_createIndex(1, 1)},
    m_models {},
    m_debug {debug}
{
    if (m_debug) DEBUG();
    m_translationUnit = clang_createTranslationUnit(m_index, m_astFilePath.c_str());
    if (m_translationUnit == NULL)
        throw TranslationUnitException();
}

Splash::~Splash()
{
    // Guard against multiple calls of this Singleton
    static auto disposed = false;
    if (disposed)
        return;

    disposed = true;

    clang_disposeTranslationUnit(m_translationUnit);
    clang_disposeIndex(m_index);
}

void Splash::printExtractedInformation()
{
    if (isDebugEnabled()) DEBUG();

    std::cout << "Found " << m_models.size() << " model(s)." << std::endl;

    for (auto& m : m_models) {
        std::cout << "Model: " << m.name << std::endl;

        for (auto& a : m.attributes) {
            std::cout << "  Attribute: " << std::endl
                      << "    Name: " << a.name << std::endl
                      << "    Description: " << a.description << std::endl
                      << "    Type: " << a.type << std::endl
                      << std::endl;
        }
    }
}

void Splash::exportExtractedInformation()
{
    if (isDebugEnabled()) {
        DEBUG();
        std::cout << "Exporting " << m_models.size() << " model(s)." << std::endl;
    }

    using namespace boost::json;
    array arr;

    for (auto& m : m_models) {
        object obj;
        array attributes;

        if (!m.parent.empty())
            obj["parent"] = m.parent;

        obj["name"] = m.name;
        for (auto& a : m.attributes) {
            attributes.push_back({
                {"name", a.name},
                {"description", a.description},
                {"type", a.type}
            });
        }
        obj["attributes"] = attributes;

        arr.push_back(obj);
    }


    std::ofstream ofs(m_outputFilePath);
    ofs << arr;
    ofs.close();
}

void Splash::extractModel(const CXCursor &cursor)
{
    if (isDebugEnabled()) DEBUG();

    auto modelName = getParentObjectName(cursor);
    m_models.push_back({modelName});
}

void Splash::exctractArgument(const CXCursor &cursor, unsigned int level)
{
    if (isDebugEnabled()) DEBUG();
    CXCursor arg;
    CXCursorKind argCurKind;
    const unsigned int expectedNumOfArguments = 3;

    for (int i = 0; i < expectedNumOfArguments; i++) {
        arg = clang_Cursor_getArgument(cursor, i);
        argCurKind = clang_getCursorKind(cursor);
        //inspectCursor(arg);

        int cd = level + 1;
        clang_visitChildren(arg, argumentExtractorCallback, &cd);
    }
}

Splash* Splash::getInstance(std::string inputPath = "", std::string outputPath = "", bool debug = false)
{
    if (m_instance == nullptr)
        m_instance = new Splash(inputPath, outputPath, debug);

    return m_instance;
}

bool Splash::isDebugEnabled()
{
    return Splash::getInstance()->m_debug;
}

bool Splash::isNamespace(const CXCursor &cursor, std::string namespaceName)
{
    if (isDebugEnabled()) DEBUG();

    CXCursorKind curKind = clang_getCursorKind(cursor);
    CXString spell = clang_getCursorSpelling(cursor);
    std::string spellText = clang_getCString(spell);
    const bool flag = curKind == CXCursorKind::CXCursor_Namespace &&
                      spellText.compare(namespaceName) == 0;
    clang_disposeString(spell);

    return flag;
}

bool Splash::isMethod(const CXCursor &cursor, std::string methodName)
{
    if (isDebugEnabled()) DEBUG();
    CXCursorKind curKind = clang_getCursorKind(cursor);
    CXString spell = clang_getCursorSpelling(cursor);
    std::string spellText = clang_getCString(spell);
    const bool flag = curKind == CXCursorKind::CXCursor_CXXMethod &&
                      spellText.compare(methodName) == 0;

    clang_disposeString(spell);

    return flag;
}

bool Splash::isTypeReference(const CXCursor &cursor)
{
    if (isDebugEnabled()) DEBUG();
    CXCursorKind curKind = clang_getCursorKind(cursor);
    const bool flag = curKind == CXCursorKind::CXCursor_TypeRef;

    return flag;
}

bool Splash::hasParent(const CXCursor &parent, std::string targetParent)
{
    if (isDebugEnabled()) DEBUG();
    CXString parentName = clang_getCursorSpelling(parent);
    const bool flag = targetParent.compare(clang_getCString(parentName)) == 0;

    return flag;
}

bool Splash::isDecl(const CXCursor &cursor, std::string typeDecl, std::string parentName)
{
    if (isDebugEnabled()) DEBUG();
    CXCursorKind curKind = clang_getCursorKind(cursor);
    CXType type = clang_getCursorType(cursor);
    CXString typeName = clang_getTypeSpelling(type);
    std::string typeNameStr = clang_getCString(typeName);
    CXCursor semanticParent = clang_getCursorSemanticParent(cursor);
    CXString semanticParentName = clang_getCursorSpelling(semanticParent);
    std::string semanticParentNameStr = clang_getCString(semanticParentName);

    const bool flag = clang_isDeclaration(curKind) &&
                      typeNameStr.compare(typeDecl) == 0 &&
                      semanticParentNameStr.compare(parentName) == 0;

    clang_disposeString(typeName);
    clang_disposeString(semanticParentName);
    return flag;
}

bool Splash::isCallExpr(const CXCursor &cursor, std::string exprReturnType, std::string name)
{
    if (isDebugEnabled()) DEBUG();
    CXCursorKind curKind = clang_getCursorKind(cursor);
    CXString curKindName = clang_getCursorKindSpelling(curKind);
    std::string curKindNameStr = clang_getCString(curKindName);
    CXType type = clang_getCursorType(cursor);
    CXString typeName = clang_getTypeSpelling(type);
    std::string typeNameStr = clang_getCString(typeName);
    CXString spell = clang_getCursorSpelling(cursor);
    std::string spellStr = clang_getCString(spell);


    const bool flag = clang_isExpression(curKind) &&
                      curKindNameStr.compare("CallExpr") == 0 &&
                      typeNameStr.compare(exprReturnType) == 0 &&
                      spellStr.compare(name) == 0;

    clang_disposeString(curKindName);
    clang_disposeString(typeName);
    clang_disposeString(spell);
    return flag;
}

bool Splash::isFromMainFile(const CXCursor &cursor)
{
    if (isDebugEnabled()) DEBUG();
    CXSourceLocation loc = clang_getCursorLocation(cursor);
    return clang_Location_isFromMainFile(loc);
}

void Splash::showSpell(const CXCursor &cursor)
{
    if (isDebugEnabled()) DEBUG();
    CXString spell = clang_getCursorSpelling(cursor);
    std::cout << "  Text: " << clang_getCString(spell) << std::endl;
    clang_disposeString(spell);
}

void Splash::showType(const CXCursor &cursor)
{
    if (isDebugEnabled()) DEBUG();
    CXType type = clang_getCursorType(cursor);
    CXString typeName = clang_getTypeSpelling(type);
    CXTypeKind typeKind = type.kind;
    CXString typeKindName = clang_getTypeKindSpelling(typeKind);
    std::cout << "  Type: " << clang_getCString(typeName) << std::endl;
    std::cout << "  TypeKind: " << clang_getCString(typeKindName) << std::endl;
    clang_disposeString(typeName);
    clang_disposeString(typeKindName);
}

void Splash::showParent(const CXCursor &cursor, const CXCursor &parent)
{
    if (isDebugEnabled()) DEBUG();
    CXCursor semaParent = clang_getCursorSemanticParent(cursor);
    CXCursor lexParent  = clang_getCursorLexicalParent(cursor);
    CXString parentName = clang_getCursorSpelling(parent);
    CXString semaParentName = clang_getCursorSpelling(semaParent);
    CXString lexParentName = clang_getCursorSpelling(lexParent);
    std::cout << "  Parent: parent:" << clang_getCString(parentName)
              << "  semantic:" << clang_getCString(semaParentName)
              << "  lexical:" << clang_getCString(lexParentName)
              << std::endl;

    clang_disposeString(parentName);
    clang_disposeString(semaParentName);
    clang_disposeString(lexParentName);
}

void Splash::showLocation(const CXCursor &cursor)
{
    if (isDebugEnabled()) DEBUG();
    CXSourceLocation loc = clang_getCursorLocation(cursor);
    CXFile file;
    unsigned line, column, offset;
    clang_getSpellingLocation(loc, &file, &line, &column, &offset);
    CXString fileName = clang_getFileName(file);
    std::cout << "Location: " << clang_getCString(fileName) << ":"
              << line << ":"
              << column << ":"
              << offset << std::endl;
    clang_disposeString(fileName);
}

void Splash::showUsr(const CXCursor &cursor)
{
    if (isDebugEnabled()) DEBUG();
    CXString usr = clang_getCursorUSR(cursor);
    std::cout << "  USR: " << clang_getCString(usr) << std::endl;
    clang_disposeString(usr);
}

void Splash::showCursorKind(const CXCursor &cursor)
{
    if (isDebugEnabled()) DEBUG();
    CXCursorKind curKind  = clang_getCursorKind(cursor);
    CXString curKindName  = clang_getCursorKindSpelling(curKind);

    const char *type;
    if (clang_isAttribute(curKind))            type = "Attribute";
    else if (clang_isDeclaration(curKind))     type = "Declaration";
    else if (clang_isExpression(curKind))      type = "Expression";
    else if (clang_isInvalid(curKind))         type = "Invalid";
    else if (clang_isPreprocessing(curKind))   type = "Preprocessing";
    else if (clang_isReference(curKind))       type = "Reference";
    else if (clang_isStatement(curKind))       type = "Statement";
    else if (clang_isTranslationUnit(curKind)) type = "TranslationUnit";
    else if (clang_isUnexposed(curKind))       type = "Unexposed";
    else                                       type = "Unknown";

    std::cout << "  CursorKind: " << clang_getCString(curKindName);
    std::cout << "  CursorKindType: " << type << std::endl;
    clang_disposeString(curKindName);
}

void Splash::showIncludedFile(const CXCursor &cursor)
{
    if (isDebugEnabled()) DEBUG();
    CXFile included = clang_getIncludedFile(cursor);
    if (included == 0)
        return;

    CXString includedFileName = clang_getFileName(included);
    std::cout << " Included file: " << clang_getCString(includedFileName);
    clang_disposeString(includedFileName);
}

void Splash::inspectCursor(const CXCursor &cursor)
{
    if (isDebugEnabled()) DEBUG();
    std::cout << "--- Cursor ---" << std::endl;
    showSpell(cursor);
    showCursorKind(cursor);
    showType(cursor);
    showLocation(cursor);
    showUsr(cursor);
    showIncludedFile(cursor);
}

void Splash::inspectCursor(const CXCursor &cursor, const CXCursor &parent)
{
    if (isDebugEnabled()) DEBUG();
    inspectCursor(cursor);
    showParent(cursor, parent);
}

std::string Splash::getParentObjectName(const CXCursor &cursor)
{
    if (isDebugEnabled()) DEBUG();
    CXCursor semanticParent = clang_getCursorSemanticParent(cursor);
    CXString semanticParentName = clang_getCursorSpelling(semanticParent);
    const std::string name {clang_getCString(semanticParentName)};

    clang_disposeString(semanticParentName);

    return name;
}

std::string Splash::getTypeName(const CXCursor &cursor)
{
    if (isDebugEnabled()) DEBUG();
    CXType type = clang_getCursorType(cursor);
    CXString typeName = clang_getTypeSpelling(type);
    const std::string typeNameStr {clang_getCString(typeName)};

    clang_disposeString(typeName);

    return typeNameStr;
}

std::string Splash::getSourceCode(const CXCursor &cursor)
{
    if (isDebugEnabled()) DEBUG();
    // Get source location to extract information
    CXSourceRange range = clang_getCursorExtent(cursor);
    CXSourceLocation startLoc = clang_getRangeStart(range);
    CXSourceLocation endLoc = clang_getRangeEnd(range);

    CXFile refFile;
    unsigned int startOffset, endOffset;

    clang_getFileLocation(startLoc, &refFile, nullptr, nullptr, &startOffset);
    clang_getFileLocation(endLoc, nullptr, nullptr, nullptr, &endOffset);

    // TODO: Macro to get std::string from a Clang operation that returns CXString
    CXString fileName = clang_getFileName(refFile);
    std::string filePath = clang_getCString(fileName);
    clang_disposeString(fileName);

    return getSourceCodeText(filePath, startOffset, endOffset);
}

std::string Splash::getSourceCodeText(std::string filePath, unsigned int startOffset, unsigned int endOffset)
{
    if (isDebugEnabled()) DEBUG();
    std::ifstream ifs(filePath);
    size_t bufferSize = endOffset - startOffset + 1;
    std::string contents(bufferSize, '\0');

    ifs.seekg(startOffset);
    ifs.read(&contents[0], bufferSize - 1);
    ifs.close();

    // cleanup string
    contents.erase(std::remove(contents.begin(), contents.end(), '\"'), contents.end());
    contents.erase(std::remove(contents.begin(), contents.end(), '\u0000'), contents.end());

    return contents;
}

CXChildVisitResult Splash::explorerCallback(CXCursor cursor, CXCursor parent, CXClientData client_data)
{
    if (isDebugEnabled()) DEBUG();
    unsigned level = *(unsigned *)client_data;
    unsigned next = level;

    // we are at the highest level of AST, check if we have an ns3 namespace
    if (level == 0 && isFromMainFile(cursor) && isNamespace(cursor, "ns3")) {
        //inspectCursor(cursor, parent);
        // visit children recursively
        next = level + 1;
    } else if (level == 1 && isMethod(cursor, "GetTypeId")) {
        //inspectCursor(cursor, parent)
        Splash::getInstance()->extractModel(cursor);

        // visit children recursively
        next = level + 1;
    } else if (level >= 2 && isDecl(cursor, "ns3::TypeId", "GetTypeId")) {
        //inspectCursor(cursor, parent)
        // visit children recursively
        next = level + 1;
    } else if (level >= 3 && isTypeReference(cursor) && hasParent(parent, "SetParent")) {
        //inspectCursor(cursor, parent);
        const std::string parentName = getTypeName(cursor);
        Splash::getInstance()->m_models.back().parent = parentName;

        // visit children recursively
        next = level + 1;
    } else if (level >= 3 && isCallExpr(cursor, "ns3::TypeId", "AddAttribute")) {
        //inspectCursor(cursor, parent)
        exctractArgument(cursor, next);

        // visit children recursively
        next = level + 1;
    }

    clang_visitChildren(cursor, explorerCallback, &next);
    return CXChildVisit_Continue;
}

CXChildVisitResult Splash::argumentExtractorCallback(CXCursor cursor, CXCursor parent, CXClientData clientData)
{
    if (isDebugEnabled()) DEBUG();
    std::string str;
    CXType type;
    CXString typeName;
    CXCursorKind curKind = clang_getCursorKind(cursor);
    auto attributeVec = &Splash::getInstance()->m_models.back().attributes;

    switch(curKind) {
        case CXCursor_StringLiteral:
            str = getSourceCode(cursor);

            if (attributeVec->empty() || !attributeVec->back().type.empty()) {
                attributeVec->push_back({});
            }

            if (attributeVec->back().name.empty()) {
                attributeVec->back().name = str;
            } else {
                attributeVec->back().description = str;
            }

            return CXChildVisit_Break;
            break;
        case CXCursor_CXXFunctionalCastExpr:
        case CXCursor_TypeRef:
            type = clang_getCursorType(cursor);
            typeName = clang_getTypeSpelling(type);
            str = clang_getCString(typeName);
            clang_disposeString(typeName);

            attributeVec->back().type = str;

            return CXChildVisit_Break;
            break;
        default:
            //inspectCursor(cursor);
            return CXChildVisit_Recurse;
    }
}

void Splash::run()
{
    if (isDebugEnabled()) DEBUG();
    unsigned level = 0;
    CXCursor cursor = clang_getTranslationUnitCursor(m_translationUnit);

    clang_visitChildren(cursor, explorerCallback, &level);
}

Splash* Splash::fromUserInput(int argc, char** argv)
{
    cxxopts::Options options("Splash", "Transpiler for IoD Sim and Airflow interoperability.");
    options.add_options()
        ("ast_file_path", "AST File Path of IoD Sim.", cxxopts::value<std::string>())
        ("output_file", "File path to write JSON output.", cxxopts::value<std::string>())
        ("d,debug", "Show debug messages.")
        ("v,version", "Show the version of the program.")
        ("h,help", "Print help");
    options.parse_positional({"ast_file_path", "output_file"});
    options.show_positional_help();

    try {
        auto result = options.parse(argc, argv);
        bool debug = false;

        if (result.count("help")) {
            std::cerr << options.help() << std::endl;
            exit(0);
        }

        if (result.count("version")) {
            std::cout << VERSION << std::endl;
        }

        if (result.count("debug")) {
            debug = true;
        }

        if (!result.count("ast_file_path")) {
            std::cerr << "AST File Path hasn't been specified." << std::endl;
            exit(1);
        }
        if (!result.count("output_file")) {
            std::cerr << "Output File Path hasn't been specified." << std::endl;
            exit(1);
        }

        std::string inputAstFilePath = result["ast_file_path"].as<std::string>();
        std::string outputFilePath = result["output_file"].as<std::string>();

        return getInstance(inputAstFilePath, outputFilePath, debug);
    } catch (cxxopts::option_not_exists_exception e) {
        std::cerr << "Error: "<< e.what() << std::endl;
        exit(1);
    }
}

int main(int argc, char** argv)
{
    try {
        auto s = Splash::fromUserInput(argc, argv);
        s->run();
        s->exportExtractedInformation();
    } catch (TranslationUnitException e) {
        std::cerr << "Cannot create translation unit." << std::endl;
        exit(1);
    }
}
