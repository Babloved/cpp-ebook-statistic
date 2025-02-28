#include <iostream>
#include <vector>
#include <algorithm>
#include <iomanip>
#include <variant>

namespace request{
    struct Cheer{
        size_t id{0};
    };

    struct Read{
        size_t id{0};
        size_t page{0};
    };

    // Тип для хранения вектора уникальных указателей на базовые запросы
    using Requests = std::vector<std::variant<Cheer, Read>>;


    // Функция для чтения запросов из потока
    Requests GetRequestsFormStream(std::istream& input, size_t count){
        Requests requests;
        for (int i = 0; i < count; ++i){
            std::string type;
            input >> type;
            if (type == "CHEER"){
                Cheer cheer;
                input >> cheer.id;
                requests.emplace_back(cheer);
            } else if (type == "READ"){
                Read read{};
                input >> read.id >> read.page;
                requests.emplace_back(read);
            }
        }
        return requests;
    }
}

namespace reader{
    // Класс для обработки запросов и поддержания статистики
    class Reader{
        using Page = size_t; // Тип для номера страницы
        using UserID = size_t; // Тип для идентификатора пользователя

    public:
        // Конструктор, инициализирующий базы данных пользователей и страниц
        Reader() : last_read_user_page_(100001, 0), count_users_on_page_(1001, 0){}

        // Метод для добавления заметки о чтении страницы пользователем
        void AddNote(UserID id, Page page){
            max_page = std::max(max_page, page);
            user_count += last_read_user_page_[id] == 0;

            for (size_t i = last_read_user_page_[id]; i < page; ++i){
                count_users_on_page_[i]++;
            }
            last_read_user_page_[id] = page;
        }

        // Метод для получения статистики чтения для пользователя
        double GetStatistic(UserID id) const{
            if (last_read_user_page_[id] == 0){
                return 0;
            }
            auto other_users_count = user_count > 1 ? user_count - 1 : 0;
            if (other_users_count == 0){
                return 1;
            }
            auto user_page = last_read_user_page_[id];
            auto less_users = count_users_on_page_.front() - count_users_on_page_[user_page - 1];
            double result = static_cast<double>(less_users) / static_cast<double>(other_users_count);
            return result;
        }

    private:
        std::vector<UserID> last_read_user_page_;
        // База данных для хранения последней прочитанной страницы каждого пользователя
        std::vector<Page> count_users_on_page_; // База данных для подсчета количества пользователей на каждой странице
        size_t max_page{0}; // Максимальный номер страницы, которую читали пользователи
        size_t user_count{0}; // Количество уникальных пользователей
    };
}

namespace request{
    struct ProcessRequest{
        void operator()(const request::Cheer cheer) const{
            std::cout << reader.GetStatistic(cheer.id) << std::endl;
        }

        void operator()(const request::Read read) const{
            reader.AddNote(read.id, read.page);
        }

        reader::Reader& reader;
    };
}

int main(){
    int count_request{0};
    std::cin >> count_request;
    auto requests = request::GetRequestsFormStream(std::cin, count_request);
    reader::Reader reader;
    for (auto& request : requests){
        std::visit(request::ProcessRequest{reader}, request);
    }
    return 0;
}