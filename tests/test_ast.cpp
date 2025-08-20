#include "bloch/ast/ast.hpp"
#include "test_framework.hpp"

using namespace bloch;

TEST(ASTTest, ProgramConstruction) {
    Program prog;
    auto imp = std::make_unique<ImportStatement>();
    imp->module = "bloch.core.math";
    prog.imports.push_back(std::move(imp));

    ASSERT_EQ(prog.imports.size(), 1);
    EXPECT_EQ(prog.imports[0]->module, "bloch.core.math");
}

TEST(ASTTest, FunctionDeclarationBasics) {
    FunctionDeclaration func;
    func.name = "foo";
    func.returnType = std::make_unique<VoidType>();
    func.body = std::make_unique<BlockStatement>();

    auto param = std::make_unique<Parameter>();
    param->name = "x";
    param->type = std::make_unique<PrimitiveType>("int");
    func.params.push_back(std::move(param));

    ASSERT_EQ(func.params.size(), 1);
    EXPECT_EQ(func.params[0]->name, "x");
    auto* prim = dynamic_cast<PrimitiveType*>(func.params[0]->type.get());
    ASSERT_NE(prim, nullptr);
    EXPECT_EQ(prim->name, "int");
}

TEST(ASTTest, ExpressionNodes) {
    auto left = std::make_unique<LiteralExpression>("1", "int");
    auto right = std::make_unique<LiteralExpression>("2", "int");
    BinaryExpression expr("+", std::move(left), std::move(right));

    EXPECT_EQ(expr.op, "+");
    auto* leftLit = dynamic_cast<LiteralExpression*>(expr.left.get());
    auto* rightLit = dynamic_cast<LiteralExpression*>(expr.right.get());
    ASSERT_NE(leftLit, nullptr);
    ASSERT_NE(rightLit, nullptr);
    EXPECT_EQ(leftLit->value, "1");
    EXPECT_EQ(rightLit->value, "2");
}