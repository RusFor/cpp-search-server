
// Решите загадку: Сколько чисел от 1 до 1000 содержат как минимум одну цифру 3?
// Напишите ответ здесь: 271
// пытался решить через строки, выдавало 886. Можно пояснить почему так? Это из за возвращаемого итератора на конец строки?
//int main(){
//    int answer = 0;
//    for(int i = 1; i < 1000; ++i){
//        if(std::to_string(i).find('3')){
//            ++answer;
//        }
//    }
//    std::cout << answer;
//    return 0;
//}


// Закомитьте изменения и отправьте их в свой репозиторий.

#include <iostream>
#include <string>

int main(){
    int answer = 0;
    for(int i = 1; i < 1000; ++i){
        int n = i;
        do{
            if(n % 10 == 3){
                ++answer;
                break;
            }
            n /= 10;
        }while(n > 0);
    }
    std::cout << answer;
    return 0;
}

