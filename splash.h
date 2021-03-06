#include <exception>
#include <string>
#include <vector>

#include <clang-c/Index.h>

// Helper structures for Splash
class TranslationUnitException : std::exception
{};

class Attribute {
public:
    Attribute() {}

    std::string name;
    std::string description;
    std::string type;
};

class Model {
public:
    Model(std::string n):
        parent {""},
        name {n},
        attributes {}
    {}

    std::string parent;
    std::string name;
    std::vector<Attribute> attributes;
};

class Splash
{
public:
    ~Splash();

    void printExtractedInformation();
    void exportExtractedInformation();
    static Splash* getInstance(std::string inputFilePath, std::string outputFilePath, bool debugMode);
    static Splash* fromUserInput(int argc, char** argv);
    static CXChildVisitResult explorerCallback(CXCursor cursor, CXCursor parent, CXClientData client_data);
    static CXChildVisitResult argumentExtractorCallback(CXCursor cursor, CXCursor parent, CXClientData clientData);

    void run();

private:
    Splash(std::string astFilePath, std::string outputPath, bool debugMode);
    void extractModel(const CXCursor &cursor);
    static void exctractArgument(const CXCursor &cursor, unsigned int level);

    static bool isDebugEnabled();
    static bool isNamespace(const CXCursor &cursor, std::string namespaceName);
    static bool isMethod(const CXCursor &cursor, std::string methodName);
    static bool isTypeReference(const CXCursor &cursor);
    static bool hasParent(const CXCursor &parent, std::string targetParent);
    static bool isDecl(const CXCursor &cursor, std::string typeDecl, std::string parentName);
    static bool isCallExpr(const CXCursor &cursor, std::string exprReturnType, std::string name);
    static bool isFromMainFile(const CXCursor &cursor);
    static void inspectCursor(const CXCursor &cursor);
    static void inspectCursor(const CXCursor &cursor, const CXCursor &parent);
    static void showSpell(const CXCursor &cursor);
    static void showType(const CXCursor &cursor);
    static void showLocation(const CXCursor &cursor);
    static void showUsr(const CXCursor &cursor);
    static void showCursorKind(const CXCursor &cursor);
    static void showIncludedFile(const CXCursor &cursor);
    static void showParent(const CXCursor &cursor, const CXCursor &parent);
    static std::string getParentObjectName(const CXCursor &cursor);
    static std::string getTypeName(const CXCursor &cursor);
    static std::string getSourceCode(const CXCursor &cursor);
    static std::string getSourceCodeText(std::string filePath, unsigned int startOffset, unsigned int endOffset);

    static Splash* m_instance;
    std::string m_astFilePath;
    std::string m_outputFilePath;
    CXTranslationUnit m_translationUnit;
    CXIndex m_index;
    std::vector<Model> m_models;
    bool m_debug;
};
