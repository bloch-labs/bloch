// Copyright 2025 Akshay Pal (https://bloch-labs.com)
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "bloch/ast/ast.hpp"
#include "test_framework.hpp"

using namespace bloch;

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
