#include <iostream>
#include <string>
#include <vector>
#include <cstring>
#include <array>
#include <algorithm>
#include <execution>

#include <regex>
#include <map>

#include <chrono>

std::string derivative_1(const std::string &polynomial) {
    struct token_t {
        long digit;
        long power;
        char value;
        char sign;

        bool only_digit;
    };
    std::vector<token_t> tokens;
    std::vector<std::string> str_tokens;
    constexpr std::array<char, 2> token_delimiter = {'+', '-'};

    size_t token_begin_index = 0;
    for (size_t i = 0; i < polynomial.size(); ++i) {
        const auto result = std::find(token_delimiter.begin(), token_delimiter.end(), polynomial[i]);
        if(result != token_delimiter.end()) {
            str_tokens.push_back(polynomial.substr(token_begin_index, i - token_begin_index));
            token_begin_index = i;
        }
    }
    str_tokens.push_back(polynomial.substr(token_begin_index, polynomial.size()));
    if(str_tokens.empty()) {
        return std::string("0");
    }
    if(str_tokens.at(0).empty()) {
        str_tokens.erase(str_tokens.cbegin());
    }

    std::for_each(std::execution::par_unseq, str_tokens.begin(), str_tokens.end(), [&tokens](std::string &token) {
        long power = 0;
        long digit = 1;
        char value = 'x';
        auto sign = token.at(0) == '-' ? '-' : '+';
        auto alpha_iterator = std::find_if(std::begin(token), std::end(token),
                [](char c){ return std::isalpha(static_cast<int>(c));});
        if (alpha_iterator != std::end(token)) {
            value = *alpha_iterator;
            auto power_iterator = std::find(token.begin(), token.end(), '^');
            if (power_iterator != std::end(token)) {
                power = std::strtol(&(*(power_iterator + 1)), nullptr, 10);
            }
            else {
                power = 1;
            }
        }
        if(std::isdigit(token.at(0))) {
            auto digit_iterator = std::begin(token);
            digit = std::strtol(&(*digit_iterator), nullptr, 10);
        }
        else if (token.size() > 1 && std::isdigit(token.at(1))) {
            auto digit_iterator = std::begin(token);
            std::advance(digit_iterator, 1);
            digit = std::strtol(&(*digit_iterator), nullptr, 10);
        }
        tokens.push_back({digit, power, value, sign, false});
    });

    std::for_each(std::execution::par_unseq, tokens.begin(), tokens.end(), [](token_t &token) {
        token.digit *= token.power;
        token.power--;
        if (token.power == 0) {
            token.only_digit = true;
        }
    });

    auto first_only_digit = std::find_if(std::begin(tokens), std::end(tokens),
            [](token_t &t){ return t.only_digit; });
    if(first_only_digit != std::end(tokens) && first_only_digit != std::end(tokens) - 1) {
        for(auto it = tokens.end() - 1; it != first_only_digit; --it) {
            if (it->only_digit){
                if (it->sign == '+') {
                    first_only_digit->digit += it->digit;
                }
                else {
                    first_only_digit->digit -= it->digit;
                }
                tokens.erase(it);
            }
        }
    }

    std::sort(tokens.begin(), tokens.end(), [](token_t &a, token_t &b) {return a.power > b.power;});

    std::string derivative;
    for(auto &token : tokens) {
        if (token.digit == 0) {
            continue;
        }
        derivative += token.sign;
        if (token.digit != 1 || token.power == 0) {
            derivative += std::to_string(token.digit);
        }
        if (token.digit != 1 && token.power != 0) {
            derivative += '*';
        }
        if (token.power != 0) {
            derivative += token.value;
        }
        else continue;
        if (token.power != 1) {
            derivative += '^';
            derivative += std::to_string(token.power);
        }
    }
    if (derivative.at(0) == '+') {
        derivative.erase(0, 1);
    }
    return derivative;
}

std::string derivative_2(std::string polynomial) {
    std::map<int, int> m;
    std::array<std::string, 3> ss;   // 0-коэффициент, 1-степень, 2-результат

    for (int i = 0; i < polynomial.length(); ss[0] = "", ss[1] = "") {
        char cSign = polynomial[i] == '+' || polynomial[i] == '-'? polynomial[i++]: '+';
        // Коэффициент
        while (isdigit(polynomial[i]))
            ss[0] += polynomial[i++];
        // Пропускаем
        if (polynomial[i] == '*')
            ++i;

        if (polynomial[i] == 'x'){
            ++i;
            // Степень
            if (polynomial[i] == '^') {
                i++;
                while (isdigit(polynomial[i])) {
                    ss[1] += polynomial[i++];
                }
            }
            m[std::atoi((ss[1].empty()?"1":ss[1]).c_str())]+=std::atoi((cSign+(ss[0].empty()?"1": ss[0])).c_str());
        }
    }
    // Раскручиваем map
    for (std::map<int, int>::reverse_iterator it = m.rbegin(); it != m.rend(); ++it) {
        std::array<int, 2> kk = { it->first - 1, it->first * it->second };   // 0-степень, 1-коэффициент

        if (it != m.rbegin() && kk[1] > 0)
            ss[2] += "+";
        // Коэффмцмент
        if (kk[0] > 0){
            if (kk[1] > 1 || kk[1] < -1)
                ss[2] += std::to_string(kk[1]) + "*";
            ss[2] += "x";
        }
        else
            ss[2] += std::to_string(kk[1]);
        // Степень
        if (kk[0] > 1)
            ss[2] += "^" + std::to_string(kk[0]);
    }
    return ss[2];
}


int main() {
    using namespace std::chrono;

    std::vector<std::string> test_data = {"x^2+x", "2*x^100+100*x^2", "x+x+x+x+x+x+x+x+x+x", "x^10000+x+1", "-x^2-x^3"};

    auto start_1 = high_resolution_clock::now();
    for(int i = 0; i < 1000; ++i) {
        for(auto &expression : test_data) {
            derivative_1(expression);
        }
    }
    auto stop_1 = high_resolution_clock::now();

    auto start_2 = high_resolution_clock::now();
    for(int i = 0; i < 1000; ++i) {
        for(auto &expression : test_data) {
            derivative_2(expression);
        }
    }
    auto stop_2 = high_resolution_clock::now();

    auto duration_1 = duration_cast<microseconds>(stop_1 - start_1);
    auto duration_2 = duration_cast<microseconds>(stop_2 - start_2);

    std::cout << "Duration 1: " << duration_1.count() << std::endl;
    std::cout << "Duration 2: " << duration_2.count() << std::endl;

    return 0;
}

