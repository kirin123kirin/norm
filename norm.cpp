#include "argparser.hpp"
#include <windows.h>

int wmain(int argc, wchar_t* argv[]) {
    std::wcout.imbue(std::locale("Japanese", std::locale::ctype));
    setlocale(LC_ALL, "");

    int idx = 0;
    const wchar_t delim = L'\t';
    const wchar_t* lineterminator = L"\r\n";
    bool strict = false;

    auto ap = ArgParser(L"Excel表で作成されたマトリクステーブルをコピーして正規化後の表を貼り付け（クリップボードに入れる）できる状態で終了します.\n引数は何番目までのカラムを残すか列番号を指定します", argc, argv);
    ap.add(L"-s", L"--strict", &strict, L"マトリクス内の値が空白の場合であっても正規化する\n");

    ap.parse();

    if(ap.positional_argv.size() != 1) {
        std::wcerr << L"引数は少なくとも一つなければなりません。何番目までのカラムを残すか列番号を指定します" << std::endl;
        return 1;
    }

    if((idx = std::stoi(ap.positional_argv[0])) == 0) {
        std::wcerr << L"引数は数値で入れてください" << std::endl;
        return 1;
    }

    if(OpenClipboard(nullptr) == false) {
        std::wcerr << L"クリップボードを開けませんでした。" << std::endl;
        return 1;
    }

    HANDLE hData = GetClipboardData(CF_UNICODETEXT);
    std::wstring ret;

    if (hData != nullptr) {
        wchar_t* txt = static_cast<wchar_t*>(GlobalLock(hData));
        if(txt != nullptr) {
            GlobalUnlock(hData);
            CloseClipboard();

            const wchar_t* p = txt;
            int i = 0;
            std::vector<std::wstring> header;
            std::wstring row, tmp;

            /* make header */
            while(*p) {
                if(*p == L'\r')
                    ++p;
                if(*p == delim) {
                    ++p;
                    ++i;
                } else if(*p == '\n') {
                    ++p;
                    break;
                }
                if(header.empty() || header.size() - 1 < i)
                    header.push_back({});
                header[i].push_back(*p++);
            }

            /* compute normalize */
            i = 0;
            while(*p) {
                if(*p == L'\r')
                    ++p;
                if(*p == L'\n') {
                    ret.append(row + lineterminator);
                    row.clear();
                    i = 0;
                } else if(*p == delim) {
                    row.push_back(*p);
                    if(idx <= i) {
                        if(strict == false) {
                            while(*(p + 1) == delim)
                                ++p;
                        }
                        if(idx == i)
                            tmp = row;
                        else
                            row.append(lineterminator + tmp);
                        
                        if(idx < i || *(p + 1) != L'\r')
                            row.append(header[i + 1] + delim);
                    }
                    ++i;
                } else {
                    row.push_back(*p);
                }
                ++p;
            }
            GlobalUnlock(hData);

        } else {
            std::wcerr << L"クリップボードのテキストを取得できませんでした。" << std::endl;
        }
    }
    CloseClipboard();

    if(ret.empty()){
        std::wcerr << L"正規化後のデータがありません" << std::endl;
        return 1;
    }

    // paste clipboard
    if (OpenClipboard(nullptr) == false) {
        std::wcerr << L"クリップボードを開けませんでした。" << std::endl;
        return 1;
    }

    EmptyClipboard();
    size_t textLength = (ret.size() + 1) * sizeof(wchar_t);

    hData = GlobalAlloc(GMEM_MOVEABLE, (ret.size() + 1) * sizeof(wchar_t));
    if (hData != nullptr) {
        wchar_t* pData = static_cast<wchar_t*>(GlobalLock(hData));
        wcscpy_s(pData, ret.size() + 1, ret.data());
        GlobalUnlock(hData);

        SetClipboardData(CF_UNICODETEXT, hData);
    } else {
        std::wcerr << L"クリップボードデータのメモリ確保に失敗しました。" << std::endl;
    }

    CloseClipboard();
    if (hData != nullptr)
        std::wcerr << L"クリップボードにデータを格納しました。Excelに貼り付けてください" << std::endl;

    return 0;
}
