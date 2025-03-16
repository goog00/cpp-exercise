#include <iostream>
#include <memory>
#include <string>


class Stmt {
    public:
        virtual ~Stmt() = default;
};

class ForNode : public Stmt {
    public:
        std::string loop_var;
        int extent;
        std::shared_ptr<Stmt> body;

        ForNode(std::string var, int ext, std::shared_ptr<Stmt> b) :
                loop_var(var), extent(ext), body(b) {}

};


class VectorizedStmt : public Stmt {

public:
     std::string loop_var;
     int extent;

     VectorizedStmt(std::string var, int ext)
            : loop_var(var), extent(ext) {}


};

class LoopVectorizer {

public:
    explicit LoopVectorizer(std::string attrs) : attributes(attrs) {}


    std::shared_ptr<Stmt> operator() (std::shared_ptr<Stmt> stmt) {
        if (auto for_node = std::dynamic_pointer_cast<ForNode>(stmt)) {
            return Vectorize(for_node);
        }
        return stmt;

    }


private:
    std::string attributes;
    std::shared_ptr<Stmt> Vectorize(std::shared_ptr<ForNode> for_node) {
        std::cout << "Vectorizing loop with variable: " << for_node->loop_var 
                  << " and extent: " << for_node->extent << " using attributes: " << attributes << std::endl;
        return std::make_shared<VectorizedStmt>(for_node->loop_var, for_node->extent);
    }
};


int main() {
    auto loop_body = std::make_shared<Stmt>();
    auto for_node = std::make_shared<ForNode>("i",4,loop_body);

    // 创建 LoopVectorizer 实例并向量化
    // LoopVectorizer vectorizer("example_attr");
    // auto vectorized_stmt = vectorizer(std::move(for_node));  // 模仿调用方式

    auto vectorized_stmt = LoopVectorizer("example_attr")(std::move(for_node));
    // 输出结果
    if (auto vec_stmt = std::dynamic_pointer_cast<VectorizedStmt>(vectorized_stmt)) {
        std::cout << "Successfully vectorized: " << vec_stmt->loop_var 
                  << " with extent: " << vec_stmt->extent << std::endl;
    }

    return 0;


}