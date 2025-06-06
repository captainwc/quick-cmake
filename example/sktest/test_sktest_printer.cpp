#include <list>
#include <map>
#include <string_view>
#include <vector>

#include "skutils/logger.h"
#include "skutils/printer.h"
#include "skutils/string_utils.h"
#include "skutils/test.h"

struct People {};

struct Person {
    int              age;   // NOLINT
    char             sex;   // NOLINT
    std::string_view name;  // NOLINT

    [[nodiscard]] std::string toString() const {
        std::stringstream ss;
        ss << "[" << name << ELEM_SEP << age << ELEM_SEP << (sex == 'm' ? "male" : "female") << "]";
        return ss.str();
    }
};

int main() {
    std::vector<std::vector<int>>   vc{{1, 2}, {3, 4}};
    std::map<int, std::vector<int>> mp{{1, {1, 2}}, {2, {2, 3}}};
    std::list<int>                  lst{1, 2, 3, 4, 5};

    LINE_BREAKER("test utils test");
    ASSERT_STR_EQUAL(REPLACED_SEP("[[1,2],[3,4]]"), sk::utils::toString(vc));
    ASSERT_STR_EQUAL(REPLACED_SEP("[{1,[1,2]},{2,[2,3]}]"), sk::utils::toString(mp));
    ASSERT_STR_EQUAL(REPLACED_SEP("[1,2,3,4,5]"), sk::utils::toString(lst));

    LINE_BREAKER("printer test");
    DUMP(sk::utils::toString(vc));
    sk::utils::print("{}", mp);
    Person person{.age = 18, .sex = 'm', .name = "shuaikai"};
    DUMP(sk::utils::toString(person));
    sk::utils::dump(sk::utils::toString(vc), sk::utils::toString(mp));

    LINE_BREAKER("DUMP test");
    DUMP(vc, mp, person, true);

    LINE_BREAKER("Pointer Test");
    std::vector<int>* pvc         = new std::vector<int>{1, 2, 3};
    People*           pPeople     = new People();
    Person*           pPerson     = new Person{.age = 18, .sex = 'm', .name = "shuaikai"};
    const char*       pstr        = "hello";
    const char*       pstrarr[]   = {"hi", "world"};
    int*              pint        = new int(99);
    auto              lambda_func = [](int a) { return a; };

    DUMP(*pvc, pvc, &pvc);
    DUMP(*pPerson, pPerson, &pPerson);
    DUMP(pPeople, &pPeople);
    DUMP(*pstr, pstr, &pstr);
    DUMP(**pstrarr, *pstrarr, pstrarr, &pstrarr)
    DUMP(*pint, pint, &pint);
    DUMP(*lambda_func, lambda_func, &lambda_func);

    return ASSERT_ALL_PASSED();
}
