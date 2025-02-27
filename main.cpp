#include <cstdint>
#include <iostream>
#include <utility>
#include <vector>
#include <memory>
#include <unordered_map>
#include <algorithm>
#include <iomanip>
#include <queue>

namespace request
{
    struct Base
    {
        size_t id{0};
        virtual ~Base() = default;
    };

    struct Cheer : Base
    {
    };

    // Структура для запроса типа "READ" с номером страницы
    struct Read : Base
    {
        size_t page{0};
    };

    // Тип для хранения вектора уникальных указателей на базовые запросы
    using Requests = std::vector<std::unique_ptr<Base>>;

    // Функция для чтения запросов из потока
    Requests GetRequestsFormStream(std::istream& input, size_t count)
    {
        Requests requests;
        for (int i = 0; i < count; ++i)
        {
            std::string type;
            input >> type;
            if (type == "CHEER")
            {
                Cheer cheer;
                input >> cheer.id;
                requests.push_back(std::make_unique<Cheer>(cheer));
            }
            else if (type == "READ")
            {
                Read read{};
                input >> read.id >> read.page;
                requests.push_back(std::make_unique<Read>(read));
            }
        }
        return requests;
    }
}

namespace reader
{
    // Класс для обработки запросов и поддержания статистики
    class Reader
    {
        using Page = size_t; // Тип для номера страницы
        using UserID = size_t; // Тип для идентификатора пользователя

    public:
        // Конструктор, инициализирующий базы данных пользователей и страниц
        Reader() : users_database_(100001, 0), count_users_on_page_(1001, 0)
        {
        }

        // Метод для добавления заметки о чтении страницы пользователем
        void AddNote(UserID id, Page page)
        {
            max_page = std::max(max_page, page);
            user_count += users_database_[id] == 0;

            for (size_t i = users_database_[id]; i < page; ++i)
            {
                count_users_on_page_[i]++;
            }
            users_database_[id] = page;
        }

        // Метод для получения статистики чтения для пользователя
        double GetStatistic(UserID id) const
        {
            if (users_database_[id] == 0)
            {
                return 0;
            }
            auto other_user = user_count > 1 ? user_count - 1 : 0;
            if (other_user == 0)
            {
                return 1;
            }
            auto user_page = users_database_[id];
            auto less_users = count_users_on_page_.front() - count_users_on_page_[user_page - 1];
            double result = static_cast<double>(less_users) / static_cast<double>(other_user);
            return result;
        }

    private:
        std::vector<UserID> users_database_;
        // База данных для хранения последней прочитанной страницы каждого пользователя
        std::vector<Page> count_users_on_page_; // База данных для подсчета количества пользователей на каждой странице
        size_t max_page{0}; // Максимальный номер страницы, которую читали пользователи
        size_t user_count{0}; // Количество уникальных пользователей
    };
}

int main()
{
    int count_request{0};
    std::cin >> count_request;
    auto requests = request::GetRequestsFormStream(std::cin, count_request);
    reader::Reader reader;
    for (auto& request : requests)
    {
        if (auto p_cheer = dynamic_cast<request::Cheer*>(request.get()))
        {
            std::cout << reader.GetStatistic(p_cheer->id) << std::endl;
        }
        else if (auto p_read = dynamic_cast<request::Read*>(request.get()))
        {
            reader.AddNote(p_read->id, p_read->page);
        }
    }
    return 0;
}