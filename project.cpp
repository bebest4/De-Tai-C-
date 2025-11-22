#include <SFML/Graphics.hpp>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <ctime>
#include <cstdlib>
#include <cstring>
#include <algorithm>
#include "mylib.h"
#include <optional>
#include "project.cpp"
using namespace sf;
using namespace std;

// Include các struct và hàm từ project.cpp
enum CurrentTab {
    TAB_DAUSACH,
    TAB_DOCGIA,
    TAB_MUONTRA
};
string UnicodeToUTF8(uint32_t codepoint) {
    string out;
    if (codepoint <= 0x7f)
        out.append(1, static_cast<char>(codepoint));
    else if (codepoint <= 0x7ff) {
        out.append(1, static_cast<char>(0xc0 | ((codepoint >> 6) & 0x1f)));
        out.append(1, static_cast<char>(0x80 | (codepoint & 0x3f)));
    }
    else if (codepoint <= 0xffff) {
        out.append(1, static_cast<char>(0xe0 | ((codepoint >> 12) & 0x0f)));
        out.append(1, static_cast<char>(0x80 | ((codepoint >> 6) & 0x3f)));
        out.append(1, static_cast<char>(0x80 | (codepoint & 0x3f)));
    }
    return out;
}
// ============================================================
// CLASS TABLE DISPLAY (GIAO DIỆN MỚI - DARK MODE & ZEBRA)
// ============================================================
class TableDisplay {
private:
    Font font;
    vector<vector<string>> data; // Dữ liệu bảng
    vector<string> headers;      // Tiêu đề cột
    vector<int> columnWidths;    // Độ rộng cột
    float startX, startY;
    float lineHeight;            // Chiều cao mỗi dòng
    float charWidthFactor;       // Hệ số quy đổi độ rộng
    int currentPage;
    int rowsPerPage;

public:
    TableDisplay(float x, float y, int rowsPerPage, string fontPath)
        : startX(x), startY(y), rowsPerPage(rowsPerPage), 
          lineHeight(40.0f),     // Tăng chiều cao dòng cho thoáng (40px)
          charWidthFactor(12.0f), // Hệ số ước lượng độ rộng chữ
          currentPage(0)
    {
        if (!font.openFromFile(fontPath)) {
            cout << "LOI: Khong tim thay file font!" << endl;
        }
    }

    void setHeaders(vector<string> h, vector<int> widths) {
        headers = h;
        columnWidths = widths;
    }

    void addRow(vector<string> row) {
        data.push_back(row);
    }

    void clear() {
        data.clear();
        currentPage = 0;
    }

    // Hàm vẽ hình chữ nhật nền
    void drawRect(RenderWindow &window, float x, float y, float w, float h, Color color) {
        RectangleShape rect(Vector2f(w, h));
        rect.setPosition(Vector2f(x, y));
        rect.setFillColor(color);
        window.draw(rect);
    }

    // Cắt chuỗi nếu quá dài
    string formatString(string str, int maxChar) {
        if (str.length() > maxChar) {
            return str.substr(0, maxChar - 2) + "..";
        }
        return str;
    }
    void draw(RenderWindow &window) {
        if (headers.empty()) return;

        // Tính tổng chiều rộng bảng
        float totalWidth = 0;
        for (int w : columnWidths) totalWidth += w * charWidthFactor;

        float currentY = startY;

        // --- 1. VẼ HEADER (Tiêu đề) ---
        // Vẽ nền Header màu xám đậm
        drawRect(window, startX, currentY, totalWidth, lineHeight, Color(50, 50, 60)); 

        Text textObj(font);
        textObj.setCharacterSize(18);
        
        float currentX = startX;
        for (int i = 0; i < headers.size(); i++) {
            float colW = columnWidths[i] * charWidthFactor;
            
            textObj.setString(sf::String::fromUtf8(headers[i].begin(), headers[i].end()));
            textObj.setFillColor(Color(255, 215, 0)); // Màu Vàng Gold
            textObj.setStyle(Text::Bold);
            
            // Căn giữa tiêu đề
            FloatRect textRect = textObj.getLocalBounds();
            textObj.setPosition(Vector2f(currentX + (colW - textRect.size.x) / 2.0f, currentY + 8.f));
            window.draw(textObj);
            
            // Đường kẻ dọc ngăn cách header
            RectangleShape line(Vector2f(1.f, lineHeight));
            line.setFillColor(Color(100, 100, 100));
            line.setPosition(Vector2f(currentX + colW, currentY));
            window.draw(line);

            currentX += colW;
        }

        currentY += lineHeight; // Xuống dòng để vẽ dữ liệu

        // --- 2. VẼ DỮ LIỆU ---
        int startRow = currentPage * rowsPerPage;
        int endRow = min(startRow + rowsPerPage, (int)data.size());

        for (int i = startRow; i < endRow; i++) {
            currentX = startX;
            
            // Hiệu ứng Zebra (Dòng chẵn lẻ màu khác nhau)
            if (i % 2 == 0) {
                drawRect(window, startX, currentY, totalWidth, lineHeight, Color(40, 40, 45)); // Xám nhẹ
            } else {
                drawRect(window, startX, currentY, totalWidth, lineHeight, Color(30, 30, 35)); // Tối hơn
            }

            for (int j = 0; j < data[i].size() && j < columnWidths.size(); j++) {
                float colW = columnWidths[j] * charWidthFactor;
                
                // Nội dung ô
                string content = formatString(data[i][j], columnWidths[j]);
                textObj.setString(sf::String::fromUtf8(content.begin(), content.end()));
                textObj.setFillColor(Color::White);
                textObj.setStyle(Text::Regular);
                textObj.setPosition(Vector2f(currentX + 10.f, currentY + 8.f)); // Padding left 10px
                window.draw(textObj);

                // Đường kẻ dọc mờ giữa các cột
                RectangleShape vLine(Vector2f(1.f, lineHeight));
                vLine.setFillColor(Color(60, 60, 60));
                vLine.setPosition(Vector2f(currentX + colW, currentY));
                window.draw(vLine);

                currentX += colW;
            }
            currentY += lineHeight;
        }

        // --- 3. VẼ VIỀN BAO QUANH ---
        RectangleShape border(Vector2f(totalWidth, currentY - startY));
        border.setPosition(Vector2f(startX, startY));
        border.setFillColor(Color::Transparent);
        border.setOutlineThickness(1.f);
        border.setOutlineColor(Color(100, 100, 100));
        window.draw(border);

        // --- 4. THÔNG TIN TRANG (Footer) ---
        int totalPages = (data.size() == 0) ? 1 : (data.size() + rowsPerPage - 1) / rowsPerPage;
        Text pageInfo(font);
        pageInfo.setString("Trang " + to_string(currentPage + 1) + " / " + to_string(totalPages) +
                          "  |  Tong so dong: " + to_string(data.size()));
        pageInfo.setCharacterSize(16);
        pageInfo.setFillColor(Color(0, 255, 255)); // Màu Cyan
        pageInfo.setPosition(Vector2f(startX, currentY + 10));
        window.draw(pageInfo);
    }
    
    void nextPage() {
        int maxPage = (data.size() == 0) ? 0 : (data.size() - 1) / rowsPerPage;
        if (currentPage < maxPage) currentPage++;
    }
    
    void prevPage() {
        if (currentPage > 0) currentPage--;
    }
};
// InputField.h hoặc để đầu file main
class InputField {
public:
    RectangleShape box;
    Text text;
    Text label; // <--- Biến mới để hiện chữ "HO:", "TEN:"
    string content;
    bool isFocus = false;

    // CẬP NHẬT LẠI DÒNG NÀY (Thêm tham số labelStr và const Font &font)
    InputField(float x, float y, float w, float h, string labelStr, const Font &font) 
        : text(font), label(font) // SFML 3.0: Gắn font ngay lập tức
    {
        // Setup ô nhập
        box.setPosition({x, y});
        box.setSize({w, h});
        box.setFillColor(Color::White);
        box.setOutlineThickness(2);
        box.setOutlineColor(Color(100, 100, 100));

        // Setup chữ người dùng nhập
        text.setCharacterSize(18);
        text.setFillColor(Color::Black);
        text.setPosition({x + 5.f, y + 5.f});

        // Setup Nhãn (Label) - Đây là phần mới thêm vào
        label.setString(sf::String::fromUtf8(labelStr.begin(), labelStr.end()));
        label.setCharacterSize(18);
        label.setFillColor(Color::White);
        label.setPosition({x, y - 25.f}); // Nằm phía trên ô nhập
    }

    void handleEvent(const Event& event, const RenderWindow& window) {
        // 1. Xử lý Click chuột
        if (const auto* mouseBtn = event.getIf<Event::MouseButtonPressed>()) {
            if (mouseBtn->button == Mouse::Button::Left) {
                Vector2f mousePos(static_cast<float>(mouseBtn->position.x), 
                                  static_cast<float>(mouseBtn->position.y));
                
                if (box.getGlobalBounds().contains(mousePos)) {
                    isFocus = true;
                    box.setOutlineColor(Color::Blue);
                } else {
                    isFocus = false;
                    box.setOutlineColor(Color(100, 100, 100));
                }
            }
        }

        // 2. Xử lý Nhập phím
        if (isFocus) {
            if (const auto* textEvent = event.getIf<Event::TextEntered>()) {
                if (textEvent->unicode == 8) { // Backspace (Xóa)
                    if (!content.empty()) {
                        // Xử lý xóa ký tự UTF-8 (vì 1 ký tự TV có thể là 2-3 byte)
                        while (!content.empty()) {
                            char c = content.back();
                            content.pop_back();
                            // Nếu byte đầu tiên ko phải là byte tiếp theo (10xxxxxx) thì dừng
                            if ((c & 0xC0) != 0x80) break; 
                        }
                    }
                } 
                // SỬA ĐOẠN NÀY: Chấp nhận cả tiếng Việt (Unicode > 31)
                else if (textEvent->unicode > 31) {
                    // Gọi hàm chuyển đổi đã viết ở Bước 2
                    content += UnicodeToUTF8(textEvent->unicode);
                }
                
                // SFML hỗ trợ hiển thị UTF-8 trực tiếp từ std::string
                text.setString(sf::String::fromUtf8(content.begin(), content.end()));
            }
        }
    }

    void draw(RenderWindow &window) {
        window.draw(box);
        window.draw(text);
        window.draw(label); // Vẽ thêm nhãn
    }
    
    void clear() {
        content = "";
        text.setString("");
    }
};
class Button {
public:
    RectangleShape shape;
    Text text;
    Color colorIdle;  // Màu bình thường
    Color colorHover; // Màu khi chuột chỉ vào

    // Constructor
    Button(float x, float y, float w, float h, string t, const Font& font, Color btnColor) 
        : text(font) // SFML 3.0: Gắn font ngay
    {
        // 1. Setup hình dáng nút
        shape.setPosition({x, y});
        shape.setSize({w, h});
        
        colorIdle = btnColor;
        colorHover = Color(btnColor.r + 50, btnColor.g + 50, btnColor.b + 50); // Làm sáng màu lên tí
        
        shape.setFillColor(colorIdle);
        shape.setOutlineThickness(2);
        shape.setOutlineColor(Color::White);

        // 2. Setup chữ
        text.setString(sf::String::fromUtf8(t.begin(), t.end()));
        text.setCharacterSize(20);
        text.setFillColor(Color::White);

        // 3. Căn giữa chữ vào nút (Công thức chuẩn SFML 3.0)
        FloatRect textRect = text.getLocalBounds();
        text.setOrigin({textRect.size.x / 2.0f, textRect.size.y / 2.0f});
        text.setPosition({
            x + w / 2.0f,
            y + h / 2.0f - 5.f // Trừ 5f để bù chiều cao font
        });
    }

    // Hàm kiểm tra chuột có đang click vào nút không
    bool isClicked(Vector2f mousePos) {
        return shape.getGlobalBounds().contains(mousePos);
    }

    // Hàm cập nhật hiệu ứng (đổi màu khi di chuột)
    void update(Vector2f mousePos) {
        if (shape.getGlobalBounds().contains(mousePos)) {
            shape.setFillColor(colorHover); // Sáng lên
        } else {
            shape.setFillColor(colorIdle);  // Bình thường
        }
    }

    void draw(RenderWindow& window) {
        window.draw(shape);
        window.draw(text);
    }
};

TreeDocGia FormTaoDocGia(const Font &font) {
    RenderWindow popup(VideoMode({400, 550}), L"Thêm Độc Giả", Style::Titlebar | Style::Close);
    popup.setFramerateLimit(60);
    // Tạo các ô nhập liệu
    InputField inputHo(50.f, 50.f, 300.f, 30.f, "HỌ:", font);
    InputField inputTen(50.f, 130.f, 300.f, 30.f, "TÊN:", font);
    //
    Text txtError(font);
    txtError.setCharacterSize(16);
    txtError.setFillColor(Color::Red); // Màu đỏ cho sợ
    txtError.setPosition({50.f, 460.f}); // Hiện bên dưới nút Lưu
    txtError.setString(""); // Ban đầu rỗng
    // Text hiển thị giới tính
    Text txtGender(font); // SFML 3.0: Phải đưa font vào
    txtGender.setString(L"GIỚI TÍNH:");
    txtGender.setCharacterSize(26);
    txtGender.setPosition({50.f, 200.f});
    Text txtSelected(font);
    txtSelected.setString(L"ĐANG CHỌN: NAM");
    txtSelected.setCharacterSize(26);
    txtSelected.setFillColor(Color::Green);
    txtSelected.setPosition({50.f, 290.f});
    
    int phaiSelection = 1; // 1: Nam, 2: Nu

    Button btnNam(50.f, 235.f, 100.f, 40.f, "NAM", font, Color(100, 100, 100));
    Button btnNu(170.f, 235.f, 100.f, 40.f, "NỮ", font, Color(100, 100, 100));
    auto UpdateGenderColor = [&]() {
        if (phaiSelection == 1) {
            btnNam.colorIdle = Color(0, 150, 0);      // Nam Xanh
            btnNu.colorIdle = Color(100, 100, 100);   // Nữ Xám
        } else {
            btnNam.colorIdle = Color(100, 100, 100);  // Nam Xám
            btnNu.colorIdle = Color(0, 150, 0);       // Nữ Xanh
        }
    };
    UpdateGenderColor();
    // Nút Lưu
    RectangleShape btnSave({120.f, 40.f});
    btnSave.setPosition({140.f, 400.f});
    btnSave.setFillColor(Color::Blue);
    
    Text txtBtn(font);
    txtBtn.setString("LUU");
    txtBtn.setCharacterSize(20);
    txtBtn.setPosition({160.f, 408.f});

    // VÒNG LẶP FORM (Chặn màn hình chính)
    while (popup.isOpen()) {
        // Event Loop kiểu SFML 3.0
        Vector2i pixelPos = Mouse::getPosition(popup);
        Vector2f clickpos(static_cast<float>(pixelPos.x), static_cast<float>(pixelPos.y));
        
        btnNam.update(clickpos);
        btnNu.update(clickpos);
        while (const optional event = popup.pollEvent()) {
            
            // Xử lý thoát form (Escape) hoặc đóng cửa sổ
            if (event->is<Event::Closed>()) {
                popup.close();
                return NULL;
            }
            inputHo.handleEvent(*event, popup);
            inputTen.handleEvent(*event, popup);
            // Xử lý click nút LƯU
            if (const auto* mouseBtn = event->getIf<Event::MouseButtonPressed>()) {
                if (mouseBtn->button == Mouse::Button::Left) {
                    Vector2f mousePos(static_cast<float>(mouseBtn->position.x), 
                                      static_cast<float>(mouseBtn->position.y));
                                      
                    if (btnNam.isClicked(mousePos)) {
                        phaiSelection = 1;
                        UpdateGenderColor(); // Cập nhật lại màu
                        txtSelected.setString(L"ĐANG CHỌN: NAM");
                    }
                    if (btnNu.isClicked(mousePos)) {
                        phaiSelection = 2;
                        UpdateGenderColor(); // Cập nhật lại màu
                        txtSelected.setString(L"ĐANG CHỌN: Nữ");
                    }

                    if (btnSave.getGlobalBounds().contains(mousePos)) {
                        // --- LOGIC LƯU DỮ LIỆU ---
                        TheDocGia a;
                        stringdg(inputHo.content,a.HO);
                        stringdg(inputTen.content,a.TEN);
                        if (a.HO.empty() || a.TEN.empty()) {
        
        
        txtError.setString(L"Lỗi: Tên không hợp lệ (số hoặc để trống)!");
        
        // Reset ô nhập để bắt nhập lại
        inputHo.clear();
        inputTen.clear();
        
        // Quay lại vòng lặp, không lưu, không thoát form
        continue; 
    }
                        txtError.setString("");
                        // Tính sum (từ code cũ của bạn)
                        if (phaiSelection == 1) strcpy(a.PHAI, "Nam");
                        else strcpy(a.PHAI, "Nũ");
                        srand(time(0));
                        a.MATHE = 1000 + rand() % (9000);
                        a.sum = 0;
                        for(char c : a.HO) a.sum += c;
                        for(char c : a.TEN) a.sum += c;
                        a.trangthai = 1;

                        TreeDocGia tmp = new nodeDocGia();
                        tmp->dg = a;
                        tmp->left = NULL;
                        tmp->right = NULL;
                        
                        return tmp; // Trả về node mới và thoát form
                    }
                }
            }
        }

        // VẼ FORM
        popup.clear(Color(40, 40, 40)); // Nền xám

        inputHo.draw(popup);
        inputTen.draw(popup);
        btnNam.draw(popup);
        btnNu.draw(popup);
        popup.draw(txtGender);
        popup.draw(txtSelected);
        popup.draw(btnSave);
        popup.draw(txtBtn);
        popup.draw(txtError);
        popup.display();
    }
    return NULL;
}
int FormNhapMaXoa(const Font &font,TreeDocGia a,String tieuDe) {
    // 1. Tạo cửa sổ nhỏ
    float winW = 350.f;
    float winH = 220.f;
    RenderWindow popup(VideoMode({(unsigned int)winW, (unsigned int)winH}), L"Mã Thẻ", Style::Titlebar | Style::Close);
    popup.setFramerateLimit(60);

    // 2. Giao diện
    Text title(font);
    title.setString(tieuDe);
    title.setCharacterSize(18);
    FloatRect textRect = title.getLocalBounds();
    title.setOrigin({textRect.size.x / 2.0f, textRect.size.y / 2.0f});
    title.setPosition({winW / 2.0f, 40.f});
    float inputW = 220.f;
    float inputH = 35.f;
    float inputX = (winW - inputW) / 2.0f; 
    InputField inputMa(inputX, 80.f, inputW, inputH, "", font);
    //
    Text txtError(font);
    txtError.setCharacterSize(16);
    txtError.setFillColor(Color::Red); // Màu đỏ
    // Đặt vị trí dưới nút Xóa
    txtError.setPosition({50.f, 200.f}); 
    txtError.setString(""); // Ban đầu để rỗng
    // Nút Xóa
    float btnW = 120.f;
    float btnH = 40.f;
    float btnX = (winW - btnW) / 2.0f;
    float btnY = 150.f;
    RectangleShape btnDelete({btnW, btnH});
    btnDelete.setPosition({btnX, btnY});
    btnDelete.setFillColor(Color::Red);

    Text txtBtn(font);
    txtBtn.setString(L"CẬP NHẬT");
    txtBtn.setCharacterSize(18);
    FloatRect btnTextRect = txtBtn.getLocalBounds();
    txtBtn.setOrigin({btnTextRect.size.x / 2.0f, btnTextRect.size.y / 2.0f});
    txtBtn.setPosition({
        btnX + btnW / 2.0f,  
        btnY + btnH / 2.0f - 5.f 
    });
    while (popup.isOpen()) {
        while (const optional event = popup.pollEvent()) {
            if (event->is<Event::Closed>()) { popup.close(); return -1; }
            if (const auto* key = event->getIf<Event::KeyPressed>()) {
                if (key->code == Keyboard::Key::Escape) { popup.close(); return -1; }
            }

            inputMa.handleEvent(*event, popup);

            if (const auto* mouseBtn = event->getIf<Event::MouseButtonPressed>()) {
                if (mouseBtn->button == Mouse::Button::Left) {
                    Vector2f mousePos(static_cast<float>(mouseBtn->position.x), 
                                      static_cast<float>(mouseBtn->position.y));
                    
                    if (btnDelete.getGlobalBounds().contains(mousePos)) {
                        
                        // --- LOGIC TẬN DỤNG HÀM NUMBER CŨ ---
                        
                        // 1. Gán giá trị mặc định là -1 (giả sử lỗi trước)
                        int maCanXoa = -1; 
                        
                        // 2. Gọi hàm number cũ của bạn
                        // Nếu nhập sai (chữ/rỗng) -> hàm return luôn -> maCanXoa vẫn là -1
                        // Nếu nhập đúng (123) -> hàm chạy đến cuối -> gán maCanXoa = 123
                        number(inputMa.content, maCanXoa);

                        // 3. Kiểm tra giá trị sau khi gọi
                        if (maCanXoa == -1) {
                            // TRƯỜNG HỢP LỖI (Vẫn là -1)
                            
                            // Hiện thông báo lên GUI (thay vì cout)
                            txtError.setString(L"Lỗi: Vui Lòng Nhập Số!");
                            
                            // Căn giữa dòng lỗi
                            FloatRect errRect = txtError.getLocalBounds();
                            txtError.setOrigin({errRect.size.x / 2.0f, 0});
                            txtError.setPosition({winW / 2.0f, 200.f});

                            // Xóa ô nhập để bắt nhập lại
                            inputMa.clear();

                            // Bỏ qua, không đóng form
                            continue; 
                        }
                        if (CheckMaThe(a,maCanXoa) == false) {
                            
                            // Báo lỗi "Không có mã trùng"
                            txtError.setString(L"Lỗi: Mã Thẻ Này Không Tồn Tại!");
                            
                            // Căn giữa lại dòng lỗi (vì nội dung chữ thay đổi -> độ dài đổi)
                            FloatRect errRect = txtError.getLocalBounds();
                            txtError.setOrigin({errRect.size.x / 2.0f, 0});
                            txtError.setPosition({winW / 2.0f, 200.f});

                            // Xóa ô nhập để nhập lại
                            inputMa.clear();

                            // QUAN TRỌNG: Tiếp tục vòng lặp, không đóng form
                            continue;
                        }

                        // --- TRƯỜNG HỢP THÀNH CÔNG (maCanXoa != -1) ---
                        popup.close();
                        return maCanXoa;
                    }
                }
            }
        }
        popup.clear(Color(40, 40, 40));
        popup.draw(title);
        inputMa.draw(popup);
        popup.draw(btnDelete);
        popup.draw(txtBtn);
        popup.draw(txtError);
        popup.display();
    }
    return -1;
}

bool FormSuaDocGia(const Font &font, TheDocGia &data) {
    float winW = 400.f; float winH = 450.f;
    RenderWindow popup(VideoMode({(unsigned int)winW, (unsigned int)winH}), L"Hiệu Chỉnh Độc Giả", Style::Titlebar | Style::Close);
    popup.setFramerateLimit(60);

    // 1. Setup ô nhập
    InputField inputHo(50.f, 80.f, 300.f, 30.f, "HỌ:", font);
    InputField inputTen(50.f, 160.f, 300.f, 30.f, "TÊN:", font);

    // --- QUAN TRỌNG: ĐIỀN THÔNG TIN CŨ VÀO Ô ---
    // Để khi mở lên, người dùng thấy tên cũ (ví dụ: "Nguyen Van A")
    inputHo.content = data.HO;
    inputHo.text.setString(data.HO);
    
    inputTen.content = data.TEN;
    inputTen.text.setString(data.TEN);
    // -------------------------------------------

    // 2. Setup Giới tính (Vẫn lấy từ data cũ)
    Text txtGender(font);
    txtGender.setString(L"GIỚI TÍNH");
    txtGender.setCharacterSize(16);
    txtGender.setPosition({50.f, 200.f});

    Text txtSelected(font);
    // Kiểm tra giới tính cũ để hiển thị đúng
    int phaiSelection = (strcmp(data.PHAI, "Nam") == 0) ? 1 : 2;
    Button btnNam(50.f, 235.f, 100.f, 40.f, "NAM", font, Color(100, 100, 100));
    Button btnNu(170.f, 235.f, 100.f, 40.f, "NỮ", font, Color(100, 100, 100));
    auto UpdateGenderColor = [&]() {
        if (phaiSelection == 1) {
            btnNam.colorIdle = Color(0, 150, 0);      // Nam Xanh
            btnNu.colorIdle = Color(100, 100, 100);   // Nữ Xám
        } else {
            btnNam.colorIdle = Color(100, 100, 100);  // Nam Xám
            btnNu.colorIdle = Color(0, 150, 0);       // Nữ Xanh
        }
    };
    UpdateGenderColor();
    txtSelected.setString(L"Đang Chọn: NAM" );
    txtSelected.setCharacterSize(16);
    txtSelected.setFillColor(Color::Green);
    txtSelected.setPosition({50.f, 290.f});

    // Nút Cập nhật
    RectangleShape btnSave({120.f, 40.f});
    btnSave.setPosition({140.f, 350.f});
    btnSave.setFillColor(Color::Blue);
    Text txtBtn(font); txtBtn.setString(L"Cập Nhật");
    txtBtn.setPosition({155.f, 358.f}); txtBtn.setCharacterSize(18);

    Text txtError(font); txtError.setFillColor(Color::Red); txtError.setCharacterSize(16);
    txtError.setPosition({50.f, 310.f});

    while (popup.isOpen()) {
        Vector2i pixelPos = Mouse::getPosition(popup);
        Vector2f clickpos(static_cast<float>(pixelPos.x), static_cast<float>(pixelPos.y));
        
        btnNam.update(clickpos);
        btnNu.update(clickpos);
        while (const optional event = popup.pollEvent()) {
            if (event->is<Event::Closed>()) { popup.close(); return false; }
            
            if (const auto* key = event->getIf<Event::KeyPressed>()) {
                if (key->code == Keyboard::Key::Escape) { popup.close(); return false; }
            }

            inputHo.handleEvent(*event, popup);
            inputTen.handleEvent(*event, popup);

            if (const auto* mouseBtn = event->getIf<Event::MouseButtonPressed>()) {
                if (mouseBtn->button == Mouse::Button::Left) {
                    Vector2f mousePos(static_cast<float>(mouseBtn->position.x), static_cast<float>(mouseBtn->position.y));
                    if (btnNam.isClicked(mousePos)) {
                        phaiSelection = 1;
                        UpdateGenderColor(); // Cập nhật lại màu
                        txtSelected.setString(L"ĐANG CHỌN: NAM");
                    }
                    if (btnNu.isClicked(mousePos)) {
                        phaiSelection = 2;
                        UpdateGenderColor(); // Cập nhật lại màu
                        txtSelected.setString(L"ĐANG CHỌN: Nữ");
                    }
                    if (btnSave.getGlobalBounds().contains(mousePos)) {
                        // Check lỗi nhập liệu
                        TheDocGia tempCheck; tempCheck.HO=""; tempCheck.TEN="";
                        stringdg(inputHo.content, tempCheck.HO);
                        stringdg(inputTen.content, tempCheck.TEN);

                        if (tempCheck.HO.empty() || tempCheck.TEN.empty()) {
                            txtError.setString(L"Tên Không Hợp Lệ!"); continue;
                        }

                        // --- CẬP NHẬT DỮ LIỆU MỚI VÀO BIẾN data ---
                        data.HO = tempCheck.HO;
                        data.TEN = tempCheck.TEN;
                        // TÍNH LẠI SUM
                        data.sum = 0;
                        for(char c : data.HO) data.sum += c;
                        for(char c : data.TEN) data.sum += c;
                        
                        // --- TUYỆT ĐỐI KHÔNG ĐỤNG VÀO data.trangthai ---
                        // Như vậy trạng thái cũ vẫn được bảo toàn

                        popup.close();
                        return true;
                    }
                }
            }
        }
        popup.clear(Color(40, 40, 40));
        inputHo.draw(popup); inputTen.draw(popup);
        popup.draw(txtGender); popup.draw(txtSelected);
        btnNam.draw(popup);
        btnNu.draw(popup);
        popup.draw(btnSave); popup.draw(txtBtn); popup.draw(txtError);
        popup.display();
    }
    return false;
}
// Trả về true nếu người dùng bấm XÁC NHẬN, false nếu Hủy
bool FormTraSach(const Font &font, int &outMaThe, string &outMaSach, string &outTenSach) {
    float winW = 400.f; float winH = 400.f; 
    RenderWindow popup(VideoMode({(unsigned int)winW, (unsigned int)winH}), L"Trả Sách", Style::Titlebar | Style::Close);
    popup.setFramerateLimit(60);

    // Setup 3 ô nhập liệu
    InputField inputMaThe(50.f, 50.f, 300.f, 30.f, "MÃ ĐỘC GIẢ:", font);
    InputField inputMaSach(50.f, 130.f, 300.f, 30.f, "MÃ SÁCH:", font);
    InputField inputTenSach(50.f, 210.f, 300.f, 30.f, "TÊN SÁCH:", font);

    // Nút bấm
    RectangleShape btnOk({120.f, 40.f});
    btnOk.setPosition({140.f, 300.f}); btnOk.setFillColor(Color(150, 0, 150));
    
    Text txtBtn(font); txtBtn.setString(L"XÁC NHẬN"); txtBtn.setCharacterSize(18);
    FloatRect tr = txtBtn.getLocalBounds();
    txtBtn.setOrigin({tr.size.x/2, tr.size.y/2});
    txtBtn.setPosition({200.f, 318.f});

    Text txtError(font); txtError.setFillColor(Color::Red); txtError.setCharacterSize(16);
    txtError.setPosition({50.f, 260.f});

    while (popup.isOpen()) {
        while (const optional event = popup.pollEvent()) {
            if (event->is<Event::Closed>()) { popup.close(); return false; }
            if (const auto* key = event->getIf<Event::KeyPressed>()) {
                if (key->code == Keyboard::Key::Escape) { popup.close(); return false; }
            }

            inputMaThe.handleEvent(*event, popup);
            inputMaSach.handleEvent(*event, popup);
            inputTenSach.handleEvent(*event, popup);

            if (const auto* mouseBtn = event->getIf<Event::MouseButtonPressed>()) {
                if (mouseBtn->button == Mouse::Button::Left) {
                    Vector2f mousePos(static_cast<float>(mouseBtn->position.x), static_cast<float>(mouseBtn->position.y));
                    
                    if (btnOk.getGlobalBounds().contains(mousePos)) {
                        
                        // CHECK LỖI
                        int tempMaThe = -1;
                        number(inputMaThe.content, tempMaThe); 

                        if (tempMaThe == -1) {
                            txtError.setString(L"Lỗi: Mã Thẻ phải là số!"); continue;
                        }
                        if (inputMaSach.content.empty() || inputTenSach.content.empty()) {
                            txtError.setString(L"Lỗi: Không được để trống!"); continue;
                        }
                        string tempMaSach = "";
                        chuanhoamasach(inputMaSach.content, tempMaSach);
                        string tempTenSach = "";
                        stringdg(inputTenSach.content, tempTenSach);
                        if (tempMaSach.empty()) {
                            txtError.setString(L"Lỗi: Mã Sách không hợp lệ!"); 
                            inputMaSach.clear(); // Xóa để nhập lại
                            continue;
                        }
                        if (tempTenSach.empty()) {
                            txtError.setString(L"Lỗi: Tên Sách không hợp lệ!"); 
                            inputTenSach.clear();
                            continue;
                        }
                        outMaThe = tempMaThe;
                        outMaSach = inputMaSach.content;
                        outTenSach = inputTenSach.content;

                        popup.close();
                        return true; // Báo là đã nhập xong
                    }
                }
            }
        }
        popup.clear(Color(40, 40, 40));
        inputMaThe.draw(popup); inputMaSach.draw(popup); inputTenSach.draw(popup);
        popup.draw(btnOk); popup.draw(txtBtn); popup.draw(txtError);
        popup.display();
    }
    return false;
}
bool FormMuonSach(const Font &font, int &outMaThe, string &outTenSach) {
    float winW = 400.f; float winH = 350.f; // Chiều cao vừa đủ cho 2 ô
    RenderWindow popup(VideoMode({(unsigned int)winW, (unsigned int)winH}), L"Mượn Sách", Style::Titlebar | Style::Close);
    popup.setFramerateLimit(60);

    // 1. Ô NHẬP MÃ THẺ
    InputField inputMaThe(50.f, 60.f, 300.f, 35.f, "MÃ ĐỘC GIẢ:", font);

    // 2. Ô NHẬP TÊN SÁCH
    InputField inputTenSach(50.f, 150.f, 300.f, 35.f, "TÊN SÁCH:", font);

    // Nút Xác Nhận
    RectangleShape btnOk({120.f, 40.f});
    btnOk.setPosition({140.f, 240.f}); 
    btnOk.setFillColor(Color(0, 100, 200)); // Màu xanh dương
    
    Text txtBtn(font); txtBtn.setString(L"XÁC NHẬN"); txtBtn.setCharacterSize(18);
    FloatRect tr = txtBtn.getLocalBounds();
    txtBtn.setOrigin({tr.size.x / 2.0f, tr.size.y / 2.0f});
    txtBtn.setPosition({200.f, 255.f});

    // Thông báo lỗi
    Text txtError(font); txtError.setFillColor(Color::Red); txtError.setCharacterSize(16);
    txtError.setPosition({50.f, 200.f}); txtError.setString("");

    while (popup.isOpen()) {
        while (const optional event = popup.pollEvent()) {
            if (event->is<Event::Closed>()) { popup.close(); return false; }
            if (const auto* key = event->getIf<Event::KeyPressed>()) {
                if (key->code == Keyboard::Key::Escape) { popup.close(); return false; }
            }

            inputMaThe.handleEvent(*event, popup);
            inputTenSach.handleEvent(*event, popup);

            if (const auto* mouseBtn = event->getIf<Event::MouseButtonPressed>()) {
                if (mouseBtn->button == Mouse::Button::Left) {
                    Vector2f mousePos(static_cast<float>(mouseBtn->position.x), static_cast<float>(mouseBtn->position.y));
                    
                    if (btnOk.getGlobalBounds().contains(mousePos)) {
                        
                        // --- 1. KIỂM TRA MÃ THẺ ---
                        int tempMaThe = -1;
                        number(inputMaThe.content, tempMaThe); // Hàm check số cũ của bạn

                        if (tempMaThe == -1) {
                            txtError.setString(L"Lỗi: Mã thẻ phải là số!"); continue;
                        }

                        // --- 2. KIỂM TRA TÊN SÁCH ---
                        string tempTenSach = "";
                        stringdg(inputTenSach.content, tempTenSach); // Chuẩn hóa tên sách (viết hoa đầu từ)

                        if (tempTenSach.empty()) {
                            txtError.setString(L"Lỗi: Tên sách không hợp lệ!"); 
                            inputTenSach.clear();
                            continue;
                        }

                        // --- 3. THÀNH CÔNG: GÁN RA NGOÀI ---
                        outMaThe = tempMaThe;
                        outTenSach = tempTenSach;

                        popup.close();
                        return true;
                    }
                }
            }
        }
        popup.clear(Color(40, 40, 40));
        
        inputMaThe.draw(popup);
        inputTenSach.draw(popup);
        
        popup.draw(btnOk); popup.draw(txtBtn); popup.draw(txtError);
        popup.display();
    }
    return false;
}
// Hàm hiển thị popup nhập một chuỗi (Dùng để nhập ISBN khi xóa hoặc tìm)
string FormNhapChuoi(const Font &font, string title, string label) {
    RenderWindow popup(VideoMode({400, 250}), "Nhập Liệu", Style::Titlebar | Style::Close);
    popup.setFramerateLimit(60);

    InputField input(50.f, 80.f, 300.f, 35.f, label, font);
    
    RectangleShape btnOk({120.f, 40.f});
    btnOk.setPosition({140.f, 160.f});
    btnOk.setFillColor(Color(0, 120, 215));
    
    Text txtBtn(font); 
    string txtBtnStr = "XÁC NHẬN";
    txtBtn.setString(sf::String::fromUtf8(txtBtnStr.begin(), txtBtnStr.end()));
    txtBtn.setCharacterSize(18);
    txtBtn.setPosition({155.f, 168.f});

    while (popup.isOpen()) {
        while (const optional event = popup.pollEvent()) {
            if (event->is<Event::Closed>()) { popup.close(); return ""; }
            if (const auto* key = event->getIf<Event::KeyPressed>()) {
                if (key->code == Keyboard::Key::Escape) { popup.close(); return ""; }
            }

            input.handleEvent(*event, popup);

            if (const auto* mouseBtn = event->getIf<Event::MouseButtonPressed>()) {
                if (mouseBtn->button == Mouse::Button::Left) {
                    if (btnOk.getGlobalBounds().contains(Vector2f(mouseBtn->position.x, mouseBtn->position.y))) {
                        if (!input.content.empty()) {
                            popup.close();
                            return input.content;
                        }
                    }
                }
            }
        }
        popup.clear(Color(40, 40, 40));
        Text tTitle(font);
        tTitle.setString(sf::String::fromUtf8(title.begin(), title.end()));
        tTitle.setPosition(Vector2f(10.f, 10.f));
        tTitle.setCharacterSize(20);
        popup.draw(tTitle);
        input.draw(popup);
        popup.draw(btnOk); popup.draw(txtBtn);
        popup.display();
    }
    return "";
}
// Trả về con trỏ DauSach mới nếu thêm thành công, hoặc NULL nếu hủy
// Nếu modeEdit = true, data sẽ là dữ liệu cũ để fill vào ô
DauSach* FormNhapSach(const Font &font, bool modeEdit, DauSach* dataEdit = NULL) {
    RenderWindow popup(VideoMode({500, 700}), modeEdit ? "Hiệu Chỉnh Sách" : "Thêm Đầu Sách", Style::Titlebar | Style::Close);
    popup.setFramerateLimit(60);

    // Tạo các ô input
    InputField inpISBN(50, 50, 400, 30, "ISBN (Tối đa 13 ký tự):", font);
    InputField inpTen(50, 130, 400, 30, "Tên Sách:", font);
    InputField inpTrang(50, 210, 150, 30, "SO TRANG:", font);
    InputField inpNam(250, 210, 200, 30, "NAM XB:", font);
    InputField inpTacGia(50, 290, 400, 30, "TAC GIA:", font);
    InputField inpTheLoai(50, 370, 400, 30, "THE LOAI:", font);
    
    // Chỉ dùng cho mode Thêm mới (để tạo tự động các bản sao)
    InputField inpSoLuong(50, 450, 150, 30, "SO LUONG SACH:", font); 

    Text txtErr(font); txtErr.setFillColor(Color::Red); txtErr.setCharacterSize(16); txtErr.setPosition(Vector2f(50.f, 600.f));

    // Nút Lưu
    RectangleShape btnSave(Vector2f(140.f, 45.f)); btnSave.setPosition(Vector2f(180.f, 530.f)); btnSave.setFillColor(Color::Blue);
    Text txtSave(font); txtSave.setString("LUU"); txtSave.setPosition(Vector2f(210.f, 540.f)); txtSave.setCharacterSize(20);

    // Nếu là EDIT, điền dữ liệu cũ vào
    if (modeEdit && dataEdit != NULL) {
        inpISBN.content = dataEdit->ISBN; inpISBN.text.setString(dataEdit->ISBN);
        inpTen.content = dataEdit->TENSACH; inpTen.text.setString(dataEdit->TENSACH);
        inpTrang.content = to_string(dataEdit->SOTRANG); inpTrang.text.setString(to_string(dataEdit->SOTRANG));
        inpNam.content = to_string(dataEdit->NAMXUATBAN); inpNam.text.setString(to_string(dataEdit->NAMXUATBAN));
        inpTacGia.content = dataEdit->TACGIA; inpTacGia.text.setString(dataEdit->TACGIA);
        inpTheLoai.content = dataEdit->THELOAI; inpTheLoai.text.setString(dataEdit->THELOAI);
        
        // Không cho sửa ISBN và Số lượng khi Edit
        // (Logic: input ISBN sẽ không nhận event click để focus)
    }

    while (popup.isOpen()) {
        while (const optional event = popup.pollEvent()) {
            if (event->is<Event::Closed>()) { popup.close(); return NULL; }
            if (const auto* key = event->getIf<Event::KeyPressed>()) {
                if (key->code == Keyboard::Key::Escape) { popup.close(); return NULL; }
            }

            // Xử lý input
            if (!modeEdit) inpISBN.handleEvent(*event, popup); // Chỉ cho sửa ISBN khi thêm mới
            inpTen.handleEvent(*event, popup);
            inpTrang.handleEvent(*event, popup);
            inpNam.handleEvent(*event, popup);
            inpTacGia.handleEvent(*event, popup);
            inpTheLoai.handleEvent(*event, popup);
            if (!modeEdit) inpSoLuong.handleEvent(*event, popup);

            if (const auto* mouseBtn = event->getIf<Event::MouseButtonPressed>()) {
                if (mouseBtn->button == Mouse::Button::Left) {
                     Vector2f mousePos(static_cast<float>(mouseBtn->position.x), static_cast<float>(mouseBtn->position.y));
                    
                    if (btnSave.getGlobalBounds().contains(mousePos)) {
                        // Validate
                        if (inpISBN.content.empty() || inpTen.content.empty()) {
                            txtErr.setString("Loi: ISBN va Ten khong duoc de trong!"); continue;
                        }
                        
                        DauSach* result = new DauSach();
                        result->ISBN = inpISBN.content;
                        stringdg(inpTen.content, result->TENSACH);
                        stringdg(inpTacGia.content, result->TACGIA);
                        stringdg(inpTheLoai.content, result->THELOAI);
                        
                        try {
                            result->SOTRANG = stoi(inpTrang.content);
                            result->NAMXUATBAN = stoi(inpNam.content);
                            if (!modeEdit) result->slsach = stoi(inpSoLuong.content);
                            else result->slsach = dataEdit->slsach; // Giữ nguyên số lượng cũ
                        } catch (...) {
                            txtErr.setString("Loi: So trang, Nam, So luong phai la so!");
                            delete result; continue;
                        }

                        popup.close();
                        return result;
                    }
                }
            }
        }

        popup.clear(Color(30, 30, 35));
        inpISBN.draw(popup); inpTen.draw(popup); inpTrang.draw(popup);
        inpNam.draw(popup); inpTacGia.draw(popup); inpTheLoai.draw(popup);
        if (!modeEdit) inpSoLuong.draw(popup); // Chỉ hiện ô số lượng khi thêm mới

        popup.draw(btnSave); popup.draw(txtSave); popup.draw(txtErr);
        popup.display();
    }
    return NULL;
}
// --- LOGIC WRAPPER CHO SFML ---

// 1. Thêm đầu sách vào mảng và tự tạo các bản sao sách con
void FormThemDauSach(DS_DauSach &ds, DauSach* p) {
    if (ds->n >= MAX_DAUSACH) return;
    
    // Check trùng ISBN
    for (int i=0; i<ds->n; i++) {
        if (ds->nodes[i]->ISBN == p->ISBN) {
            cout << "Loi: Trung ISBN!" << endl; // Có thể hiện thông báo GUI
            return;
        }
    }

    // Tạo tự động các sách con (Mã sách)
    // Logic lấy từ project.cpp
    for (int i = 0; i < p->slsach; i++) {
        SACH newSach = new nodeSach();
        newSach->next = NULL;
        
        string MaSach;
        do {
            MaSach = randomMaSach(p->ISBN);
        } while(MaSachTrung(p, MaSach)); // Kiểm tra trong list con của nó
        
        newSach->data.MASACH = MaSach;
        newSach->data.trangthai = 0; 
        newSach->data.vitri = "Ke chinh"; // Mặc định (có thể làm form chọn nếu muốn)
        
        // Insert Head
        newSach->next = p->dms;
        p->dms = newSach;
    }

    // Chèn vào danh sách tuyến tính (có sắp xếp theo Tên)
    int pos = ds->n;
    for (int i = 0; i < ds->n; i++) {
        if (p->TENSACH < ds->nodes[i]->TENSACH) {
            pos = i;
            break;
        }
    }
    for (int i = ds->n; i > pos; i--) {
        ds->nodes[i] = ds->nodes[i-1];
    }
    ds->nodes[pos] = p;
    ds->n++;
}

// 2. Xóa đầu sách
void FormXoaDauSach(DS_DauSach &ds, string isbn) {
    int pos = -1;
    for(int i=0; i<ds->n; i++) if(ds->nodes[i]->ISBN == isbn) { pos = i; break; }
    
    if (pos == -1) return; // Khong tim thay
    
    // Check xem có sách đang mượn không
    SACH p = ds->nodes[pos]->dms;
    while(p) {
        if (p->data.trangthai == 1) return; // Đang có người mượn -> Cấm xóa
        p = p->next;
    }

    // Xóa bộ nhớ các sách con
    SACH temp = ds->nodes[pos]->dms;
    while(temp) {
        SACH del = temp;
        temp = temp->next;
        delete del;
    }
    delete ds->nodes[pos];

    // Dồn mảng
    for(int i=pos; i<ds->n-1; i++) ds->nodes[i] = ds->nodes[i+1];
    ds->n--;
}

// 3. Thêm bản sao (Thêm 1 cuốn vào đầu sách đã có)
void FormThemBanSao(DS_DauSach &ds, string isbn, int soLuongThem) {
    int pos = -1;
    for(int i=0; i<ds->n; i++) if(ds->nodes[i]->ISBN == isbn) { pos = i; break; }
    if (pos == -1) return;

    DauSach* book = ds->nodes[pos];
    
    for(int k=0; k<soLuongThem; k++) {
        SACH newSach = new nodeSach();
        string ms;
        do { ms = randomMaSach(book->ISBN); } while(MaSachTrung(book, ms));
        
        newSach->data.MASACH = ms;
        newSach->data.trangthai = 0;
        newSach->data.vitri = "Ke kho";
        
        newSach->next = book->dms;
        book->dms = newSach;
        book->slsach++;
    }
}
void loadDauSachToTable(DS_DauSach &ds, TableDisplay &table) {
    table.clear();
    if (ds == NULL || ds->n == 0) return;
    
    for (int i = 0; i < ds->n; i++) {
        vector<string> row;
        row.push_back(to_string(i+1));
        row.push_back(ds->nodes[i]->ISBN);
        row.push_back(ds->nodes[i]->TENSACH);
        row.push_back(to_string(ds->nodes[i]->SOTRANG));
        row.push_back(ds->nodes[i]->TACGIA);
        row.push_back(to_string(ds->nodes[i]->NAMXUATBAN));
        row.push_back(ds->nodes[i]->THELOAI);
        row.push_back(to_string(ds->nodes[i]->slsach));
        table.addRow(row);
    }
}

void loadDocGiaToTable(TreeDocGia root, TableDisplay &table, int &cnt) {
    if (root == NULL) return;
    
    loadDocGiaToTable(root->left, table, cnt);
    
    vector<string> row;
    row.push_back(to_string(++cnt));
    row.push_back(to_string(root->dg.MATHE));
    row.push_back(root->dg.HO + " " + root->dg.TEN);
    row.push_back(string(root->dg.PHAI));
    row.push_back((root->dg.trangthai == 1) ? "Hoat dong" : "Khoa");
    row.push_back(to_string(root->dg.sachmuon));
    table.addRow(row);
    
    loadDocGiaToTable(root->right, table, cnt);
}
void loadMuonTraToTable(MT head, TableDisplay &table,string ten,int x) {
    MT p = head;
    bool coSach = false;
    int stt = 1;
    while(p != NULL) {

        if(p->mt.trangthai2 == 0) { 
            vector<string> row;
            row.push_back(to_string(stt++));
            row.push_back(to_string(x));
            row.push_back(ten);
            row.push_back(p->mt.MASACH);
            row.push_back(p->mt.TENSACH);
            row.push_back(p->mt.NgayMuon);
            row.push_back(p->mt.NgayTra); // Hạn trả
            row.push_back("Đang mượn");
            table.addRow(row);
            coSach = true;
        }
        p = p->next;
    }
    if (!coSach) {
       vector<string> emptyRow = {
            "-", to_string(x), ten, "Khong co sach", "dang muon", "-", "-", "-"};
            table.addRow(emptyRow);
    }
}
// Hàm hiển thị cửa sổ danh sách bản sao của 1 đầu sách
void FormXemChiTietBanSao(DauSach* d, const Font &font) {
    if (d == NULL) return;

    // 1. Tạo cửa sổ Popup
    RenderWindow popup(VideoMode({800, 600}), "Chi Tiet Ban Sao: " + d->ISBN, Style::Titlebar | Style::Close);
    popup.setFramerateLimit(60);

    // 2. Tạo bảng hiển thị
    // Đường dẫn font bạn nhớ sửa lại nếu dùng font khác
    TableDisplay table(30.f, 100.f, 10, "C:/Windows/Fonts/segoeui.ttf"); 

    // Cấu hình cột cho bảng chi tiết
    vector<string> headers = {"STT", "MÃ SÁCH", "TRẠNG THÁI", "VỊ TRÍ"};
    vector<int> widths = {5, 20, 20, 20};
    table.setHeaders(headers, widths);

    // 3. Load dữ liệu từ danh sách liên kết (d->dms) vào bảng
    SACH p = d->dms;
    int stt = 0;
    while (p != NULL) {
        vector<string> row;
        row.push_back(to_string(++stt));
        row.push_back(p->data.MASACH);
        
        // Xử lý trạng thái ra chữ tiếng Việt
        string tt;
        if (p->data.trangthai == 0) tt = "Được mượn";
        else if (p->data.trangthai == 1) tt = "Đã mượn";
        else tt = "Đã thanh lý";
        row.push_back(tt);

        row.push_back(p->data.vitri);
        
        table.addRow(row);
        p = p->next;
    }

    // Tiêu đề to ở trên
    Text title(font);
    title.setString(sf::String::fromUtf8(string("DANH SÁCH BẢN SAO: " + d->TENSACH).begin(), string("DANH SÁCH BẢN SAO: " + d->TENSACH).end()));
    title.setCharacterSize(20);
    title.setFillColor(Color::Yellow);
    title.setPosition(Vector2f(30.f, 30.f));

    // Vòng lặp cửa sổ popup
    while (popup.isOpen()) {
        while (const optional event = popup.pollEvent()) {
            if (event->is<Event::Closed>()) popup.close();
            
            // Hỗ trợ cuộn chuột hoặc phím trang cho bảng popup
            if (const auto* key = event->getIf<Event::KeyPressed>()) {
                if (key->code == Keyboard::Key::Escape) popup.close();
                if (key->code == Keyboard::Key::PageUp) table.prevPage();
                if (key->code == Keyboard::Key::PageDown) table.nextPage();
            }
        }

        popup.clear(Color(40, 40, 45)); // Màu nền tối
        popup.draw(title);
        table.draw(popup);
        popup.display();
    }
}
void ShowMessage(const Font& font, String noiDung) {
    // 1. Tạo cửa sổ nhỏ 400x200
    RenderWindow msgBox(VideoMode({450, 200}), L"Thông Báo", Style::Titlebar | Style::Close);
    msgBox.setFramerateLimit(60);

    // 2. Nội dung thông báo
    Text txtContent(font);
    txtContent.setString(noiDung);
    txtContent.setCharacterSize(18);
    txtContent.setFillColor(Color::White);
    
    // Căn giữa nội dung
    FloatRect textRect = txtContent.getLocalBounds();
    txtContent.setOrigin({textRect.size.x / 2.0f, textRect.size.y / 2.0f});
    txtContent.setPosition({225.f, 80.f}); // Giữa cửa sổ (450/2)

    // 3. Nút OK
    RectangleShape btnOK({100.f, 35.f});
    btnOK.setPosition({175.f, 140.f}); // Căn giữa (450-100)/2
    btnOK.setFillColor(Color(0, 120, 215)); // Xanh dương

    Text txtBtn(font);
    txtBtn.setString("OK");
    txtBtn.setCharacterSize(18);
    FloatRect btnRect = txtBtn.getLocalBounds();
    txtBtn.setOrigin({btnRect.size.x / 2.0f, btnRect.size.y / 2.0f});
    txtBtn.setPosition({225.f, 152.f});

    // 4. Vòng lặp chờ bấm OK
    while (msgBox.isOpen()) {
        while (const optional event = msgBox.pollEvent()) {
            // Đóng khi bấm X
            if (event->is<Event::Closed>()) msgBox.close();
            // Đóng khi Click nút OK
            if (const auto* mouseBtn = event->getIf<Event::MouseButtonPressed>()) {
                if (mouseBtn->button == Mouse::Button::Left) {
                    Vector2f mousePos(static_cast<float>(mouseBtn->position.x), 
                                      static_cast<float>(mouseBtn->position.y));
                    
                    if (btnOK.getGlobalBounds().contains(mousePos)) {
                        msgBox.close();
                    }
                }
            }
        }

        msgBox.clear(Color(50, 50, 50)); // Nền xám đậm
        msgBox.draw(txtContent);
        msgBox.draw(btnOK);
        msgBox.draw(txtBtn);
        msgBox.display();
    }
}
// Hàm tìm kiếm độc giả trong cây để lấy danh sách mượn
void timVaLoadMuonTra(TreeDocGia t, int mathe, TableDisplay &table) {
    if (t == NULL) return;
    if (t->dg.MATHE == mathe) {
        string hoten = t->dg.HO+" "+t->dg.TEN;
        loadMuonTraToTable(t->dg.dsmuontra, table,hoten,mathe);
        return;
    } else if (mathe < t->dg.MATHE) {
        timVaLoadMuonTra(t->left, mathe, table);
    } else {
        timVaLoadMuonTra(t->right, mathe, table);
    }
}
// Thêm tham số: string &thongbao để hứng nội dung thay vì cout
bool muonsach(DS_DauSach & a, TreeDocGia & b, const string s, int x, String &thongbao){
    if(b == NULL){
        thongbao = L"Lỗi: Mã độc giả không tồn tại!";
        return false; 
    }
    else if(b->dg.MATHE > x){
        return muonsach(a, b->left, s, x, thongbao); 
    }
    else if(b->dg.MATHE < x){
        return muonsach(a, b->right, s, x, thongbao); 
    }
    else {
        Date t = time();
        if(b->dg.dsmuontra != NULL){
            checkdaymt(b, t);
        };
        if(b->dg.sachmuon >= 3){
            thongbao = L"Thất bại: Độc giả đã mượn đủ 3 cuốn sách";
            return false;
        }
        if(b->dg.trangthai == 0){
             thongbao = L"Thất Bại: Thẻ độc giả này bị khóa";
             return false;
        }
        for(int i = 0 ; i < a->n; ++i){
            if(a->nodes[i]->TENSACH == s){ // Dò thấy tên sách trùng
                SACH temp = a->nodes[i]->dms;
                
                while(temp != NULL){      
                    if(temp->data.trangthai == 0){ // Tìm thấy cuốn chưa ai mượn
                        MT tmp = makeMT();
                        
                        tmp->mt.MASACH = temp->data.MASACH;
                        tmp->mt.TENSACH = a->nodes[i]->TENSACH;
                        tmp->mt.trangthai2 = 0;
                        tmp->mt.NgayMuon = to_string(t.day) + "/"+ to_string(t.month) + "/" +to_string(t.year);
                        
                
                        b->dg.sachmuon++;
                        temp->data.trangthai = 1;
                        a->nodes[i]->slm++;
                        
                        if(b->dg.dsmuontra == NULL){
                            b->dg.dsmuontra = tmp;
                        }
                        else{
                            MT p = b->dg.dsmuontra;
                            while(p->next != NULL){
                                p = p->next;
                            }
                            p->next = tmp;
                        }
                        
                        thongbao = L"Mượn Sách Thành Công " + s;
                        return true; //
                    }
                    temp = temp->next;
                }
                thongbao = L"Sách này đã hết bản sao để mượn";
                return false;
            }
        }
        thongbao = L"Sách này không tồn tại trong thư viện <3";
        return false;
    }
}
// ============================================================
// HÀM MAIN - CHƯƠNG TRÌNH CHÍNH
// ============================================================
int main() {
    // --- 1. KHỞI TẠO DỮ LIỆU ---
    DS_DauSach dsdausach = new DS_DAUSACH();
    TreeDocGia dsdocgia = NULL;
    TreeDocGia dshoten = NULL;

    // Load dữ liệu từ file
    ifstream Fout("D:/code/thedocgiadata.txt");
    if (Fout.is_open()) {
        loadfiledocgia(dsdocgia, dshoten, Fout);
        Fout.close();
    }
    ifstream FoutSach("D:/code/danhmucsachdata.txt");
    if (FoutSach.is_open()) {
        loadfilesach(dsdausach, FoutSach);
        FoutSach.close();
    }

    // --- 2. KHỞI TẠO CỬA SỔ ---
    float screenW = 1920.f;
    float screenH = 1080.f;
    RenderWindow Window(VideoMode({(unsigned int)screenW, (unsigned int)screenH}), "Thu Vien - Quan Ly (Full HD)");
    Window.setFramerateLimit(60);

    Font font;
    if (!font.openFromFile("C:/Windows/Fonts/segoeui.ttf")) {
        cout << "LOI: Khong tim thay font segoeui.ttf" << endl;
        return -1;
    }

    // --- 3. CẤU HÌNH GIAO DIỆN ---
    
    // A. BẢNG DỮ LIỆU
    // Đặt bảng dịch sang phải (x=80), hiển thị 18 dòng
    TableDisplay tableDisplay(80.f, 180.f, 15, "C:/Windows/Fonts/segoeui.ttf");

    // Cấu trúc cột (Header & Width)
    vector<string> headerDocGia = {"STT","MÃ THẺ", "HỌ VÀ TÊN", "PHÁI", "TRẠNG THÁI", "SÁCH MƯỢN"};
    vector<int> widthDocGia = {5, 12, 35, 10, 20, 12}; 

    vector<string> headerSach = {"STT","ISBN", "TÊN SÁCH", "SỐ TRANG", "TÁC GIẢ", "NĂM XUẤT BẢN", "THỂ LOẠI", "TỔNG BẢN SAO"};
    vector<int> widthSach = {5, 15, 40, 10, 25, 14, 18, 14}; 

    vector<string> headerMuon = {"STT","MÃ ĐG", "HỌ TÊN","MÃ SÁCH", "TÊN SÁCH", "NGÀY MƯỢN", "HẠN TRẢ", "TRẠNG THÁI"};
    vector<int> widthMuon = {5,10,25,15,30,12,12,15};

    // B. MENU TAB (Trên cùng)
    float tabW = 350.f, tabH = 50.f, tabY = 50.f, tabGap = 20.f;
    float totalTabsW = (tabW * 3) + (tabGap * 2);
    float startTabX = (screenW - totalTabsW) / 2.0f; // Căn giữa màn hình

    Color colTabIdle(60, 60, 70);   // Xám
    Color colTabActive(0, 120, 215); // Xanh dương đậm
    Color colTabHover(0, 128, 0); // Xanh lá 

    Button btnTabSach(startTabX, tabY, tabW, tabH, "DANH MỤC SÁCH", font, colTabIdle);
    Button btnTabDocGia(startTabX + tabW + tabGap, tabY, tabW, tabH, "ĐỘC GIẢ", font, colTabActive);
    Button btnTabMuon(startTabX + (tabW + tabGap)*2, tabY, tabW, tabH, "MƯỢN - TRẢ", font, colTabIdle);

    // C. NÚT CHỨC NĂNG (ĐÃ CHUYỂN XUỐNG DƯỚI ĐÁY)
        
    // Cấu hình kích thước và vị trí
    float actionW = 250.f;       // Giữ nguyên chiều rộng nút
    float actionH = 55.f;        // Chiều cao nút (nhỏ hơn xíu cho gọn)
    float actionY = 870.f;       // Tọa độ Y cố định ở dưới cùng (Dưới bảng và nút phân trang)
    float startActionX = 80.f;   // Bắt đầu từ lề trái (thẳng hàng với bảng)
    float actionGap = 30.f;      // Khoảng cách giữa các nút

    // Tính toán tọa độ X cho 4 vị trí nằm ngang
    float x1 = startActionX;
    float x2 = startActionX + actionW + actionGap;
    float x3 = startActionX + (actionW + actionGap) * 2;
    float x4 = startActionX + (actionW + actionGap) * 3;
    float x5 = startActionX + (actionW + actionGap) * 4;

    Color colBtnAction(46, 139, 87); // Màu SeaGreen (Dịu mắt)

    // --- NHÓM ĐỘC GIẢ (Tab DocGia) ---
    // Sắp xếp ngang: Thêm -> Xóa -> Sửa -> Khóa
    Button btnAdd(x1, actionY, actionW, actionH, "THÊM ĐỘC GIẢ", font, colBtnAction);
    Button btnDel(x2, actionY, actionW, actionH, "XOÁ ĐỘC GIẢ", font, colBtnAction);
    Button btnEdit(x3, actionY, actionW, actionH, "HIỆU CHỈNH", font, colBtnAction);
    Button btnlock(x4, actionY, actionW, actionH, "KHOÁ / MỞ KHOÁ", font, colBtnAction);

    // --- NHÓM SÁCH (Tab DauSach) ---
    // Sắp xếp ngang: Thêm -> Xóa -> Sửa -> Thêm Bản Sao
    Button btnSach_Them(x1, actionY, actionW, actionH, "THÊM SÁCH", font, colBtnAction);
    Button btnSach_Xoa(x2, actionY, actionW, actionH, "XOÁ SÁCH", font, colBtnAction);
    Button btnSach_DieuChinh(x3, actionY, actionW, actionH, "SỬA THÔNG TIN", font, colBtnAction);
    Button btnSach_ThemBanSao(x4, actionY, actionW, actionH, "THÊM BẢN SAO", font, colBtnAction);
    Button btnSach_ChiTietBanSao(x5, actionY, actionW, actionH, "CHI TIẾT BẢN SAO", font, colBtnAction);

    // --- NHÓM MƯỢN TRẢ (Tab MuonTra) ---
    // Sắp xếp ngang: Xem Chi Tiết -> Trả Sách
    Button btnMuon_in(x1, actionY, actionW, actionH, "XEM CHI TIẾT", font, colBtnAction);
    Button btnMuon_Tra(x2, actionY, actionW, actionH, "TRẢ SÁCH", font, colBtnAction);
    Button btnMuon_Muon(x3, actionY, actionW, actionH, "CHO MƯỢN", font, colBtnAction);

    // D. NÚT PHÂN TRANG (Dưới bảng)
    // Y = 180 (start bảng) + (18 dòng * 40px) = 900 => Đặt nút ở 950
    float pageBtnY = 950.f;
    Button btnPrevPage(80.f, pageBtnY, 150.f, 40.f, "<< TRƯỚC", font, Color(100, 100, 100));
    Button btnNextPage(240.f, pageBtnY, 150.f, 40.f, "SAU >>", font, Color(100, 100, 100));

    // --- 4. LOGIC ĐIỀU KHIỂN ---
    CurrentTab currentTab = TAB_DOCGIA;
    int currentViewMaThe = -1; // Lưu mã thẻ đang xem chi tiết

    // Hàm làm mới dữ liệu bảng
    // --- LAMBDA REFRESH LIST (Cập nhật giao diện) ---
    auto RefreshList = [&]() {
        tableDisplay.clear();

        // 1. Reset màu sắc cho cả 3 Tab
        // Trạng thái nghỉ (Idle) = Xám
        btnTabSach.colorIdle = colTabIdle;
        btnTabDocGia.colorIdle = colTabIdle;
        btnTabMuon.colorIdle = colTabIdle;

        // Trạng thái di chuột (Hover) = XANH SÁNG (Cho tất cả các tab)
        // Dòng này giúp tab chưa chọn cũng hiện xanh khi hover
        btnTabSach.colorHover = colTabHover;
        btnTabDocGia.colorHover = colTabHover;
        btnTabMuon.colorHover = colTabHover;

        // 2. Xử lý Tab đang chọn (Active)
        if (currentTab == TAB_DAUSACH) {
            btnTabSach.colorIdle = colTabActive; // Đang chọn thì màu nền là Xanh Đậm
            tableDisplay.setHeaders(headerSach, widthSach);
            loadDauSachToTable(dsdausach, tableDisplay);
        }
        else if (currentTab == TAB_DOCGIA) {
            btnTabDocGia.colorIdle = colTabActive;
            tableDisplay.setHeaders(headerDocGia, widthDocGia);
            int stt = 0;
            loadDocGiaToTable(dsdocgia, tableDisplay,stt);
        }
        else if (currentTab == TAB_MUONTRA) {
            btnTabMuon.colorIdle = colTabActive;
            tableDisplay.setHeaders(headerMuon, widthMuon);
            
            if (currentViewMaThe != -1) {
                timVaLoadMuonTra(dsdocgia, currentViewMaThe, tableDisplay);
            } else {
            }
        }
    };

    RefreshList(); // Gọi lần đầu

    // --- 5. VÒNG LẶP CHÍNH ---
    while (Window.isOpen()) {
        // Cập nhật hiệu ứng Hover chuột
        Vector2i pixelPos = Mouse::getPosition(Window);
        Vector2f mousePos(static_cast<float>(pixelPos.x), static_cast<float>(pixelPos.y));

        btnTabSach.update(mousePos); btnTabDocGia.update(mousePos); btnTabMuon.update(mousePos);
        btnPrevPage.update(mousePos); btnNextPage.update(mousePos);

        if (currentTab == TAB_DOCGIA) {
            btnAdd.update(mousePos); btnDel.update(mousePos); btnEdit.update(mousePos); btnlock.update(mousePos);
        }
        else if (currentTab == TAB_DAUSACH) {
            btnSach_Them.update(mousePos); btnSach_Xoa.update(mousePos); 
            btnSach_DieuChinh.update(mousePos); btnSach_ThemBanSao.update(mousePos);
            btnSach_ChiTietBanSao.update(mousePos);
        }
        else if (currentTab == TAB_MUONTRA) {
            btnMuon_in.update(mousePos); btnMuon_Tra.update(mousePos);btnMuon_Muon.update(mousePos);
        }

        // Xử lý sự kiện
        while (const optional event = Window.pollEvent()) {
            if (event->is<Event::Closed>()) Window.close();
            
            // Phím tắt chuyển trang
            if (const auto* key = event->getIf<Event::KeyPressed>()) {
                if (key->code == Keyboard::Key::PageUp) tableDisplay.prevPage();
                if (key->code == Keyboard::Key::PageDown) tableDisplay.nextPage();
            }

            if (const auto* mouseBtn = event->getIf<Event::MouseButtonPressed>()) {
                if (mouseBtn->button == Mouse::Button::Left) {
                    Vector2f clickPos(static_cast<float>(mouseBtn->position.x), static_cast<float>(mouseBtn->position.y));
                    
                    // -- XỬ LÝ CLICK TAB --
                    if (btnTabDocGia.isClicked(clickPos)) { currentTab = TAB_DOCGIA; RefreshList(); }
                    if (btnTabSach.isClicked(clickPos)) { currentTab = TAB_DAUSACH; RefreshList(); }
                    if (btnTabMuon.isClicked(clickPos)) { currentTab = TAB_MUONTRA; currentViewMaThe = -1; RefreshList(); }
                    
                    // -- XỬ LÝ CLICK PHÂN TRANG --
                    if (btnPrevPage.isClicked(clickPos)) tableDisplay.prevPage();
                    if (btnNextPage.isClicked(clickPos)) tableDisplay.nextPage();

                    // -- XỬ LÝ CLICK CHỨC NĂNG --
                    if (currentTab == TAB_DOCGIA) {
                        if (btnAdd.isClicked(clickPos)) {
                            TreeDocGia newNode = FormTaoDocGia(font);
                            if (newNode != NULL) {
                                caythedocgia(dsdocgia, newNode);
                                RefreshList(); 
                            }
                        }
                        if (btnDel.isClicked(clickPos)) {
                            int maXoa = FormNhapMaXoa(font, dsdocgia, L"NHẬP MÃ CẦN XÓA:");
                            if (maXoa != -1) {
                                xoathe(dsdocgia, maXoa);
                                RefreshList();
                            }
                        }
                        if (btnEdit.isClicked(clickPos)) {
                            int maSua = FormNhapMaXoa(font, dsdocgia, L"NHẬP MÃ CẦN SỬA:");
                            if (maSua != -1) {
                                TheDocGia dgMoi; dgMoi.MATHE = maSua;
                                if(FormSuaDocGia(font, dgMoi)) {
                                    dieuchinhmathe(dsdocgia, dgMoi);
                                    RefreshList();
                                }
                            }
                        }
                        if (btnlock.isClicked(clickPos)) {
                            int maKhoa = FormNhapMaXoa(font, dsdocgia, L"NHẬP MÃ CẦN KHÓA:");
                            if (maKhoa != -1) {
                                khoathe(dsdocgia, maKhoa);
                                RefreshList();
                            }
                        }
                    }
                    else if (currentTab == TAB_DAUSACH) {
                        // 1. THEM DAU SACH
                        if (btnSach_Them.isClicked(clickPos)) {
                            DauSach* pMoi = FormNhapSach(font, false); // false = mode Add
                            if (pMoi != NULL) {
                                FormThemDauSach(dsdausach, pMoi);
                                RefreshList();
                            }
                        }
                        // 2. XOA DAU SACH
                        if (btnSach_Xoa.isClicked(clickPos)) {
                            string isbn = FormNhapChuoi(font, "NHẬP ISBN CẦN XOÁ:", "ISBN:");
                            if (!isbn.empty()) {
                                FormXoaDauSach(dsdausach, isbn);
                                RefreshList();
                            }
                        }
                        // 3. SUA THONG TIN
                        if (btnSach_DieuChinh.isClicked(clickPos)) {
                            string isbn = FormNhapChuoi(font, "NHẬP ISBN CẦN SỬA:", "ISBN:");
                            if (!isbn.empty()) {
                                // Tìm sách để lấy dữ liệu cũ
                                DauSach* oldData = NULL;
                                int idx = -1;
                                for(int i=0; i<dsdausach->n; i++) {
                                    if(dsdausach->nodes[i]->ISBN == isbn) {
                                        oldData = dsdausach->nodes[i];
                                        idx = i;
                                        break;
                                    }
                                }
                                
                                if (oldData != NULL) {
                                    DauSach* newData = FormNhapSach(font, true, oldData); // true = mode Edit
                                    if (newData != NULL) {
                                        // Cập nhật lại thông tin
                                        oldData->TENSACH = newData->TENSACH;
                                        oldData->TACGIA = newData->TACGIA;
                                        oldData->NAMXUATBAN = newData->NAMXUATBAN;
                                        oldData->SOTRANG = newData->SOTRANG;
                                        oldData->THELOAI = newData->THELOAI;
                                        
                                        // Nếu tên đổi, cần sort lại (đơn giản nhất là xóa đi add lại con trỏ, 
                                        // nhưng ở đây tạm thời ta chỉ cập nhật giá trị để tránh lỗi pointer)
                                        delete newData; // Xóa object tạm
                                        RefreshList();
                                    }
                                }
                            }
                        }
                        // 4. THEM BAN SAO
                        if (btnSach_ThemBanSao.isClicked(clickPos)) {
                            string isbn = FormNhapChuoi(font, "NHẬP ISBN ĐỂ THÊM BẢN SAO:", "ISBN:");
                            if (!isbn.empty()) {
                                // Mặc định thêm 1 cuốn, nếu muốn nhiều hơn có thể làm form nhập số
                                FormThemBanSao(dsdausach, isbn, 1);
                                RefreshList();
                            }
                        }
                        // --- CHỨC NĂNG MỚI: CHI TIẾT BẢN SAO ---
                        if (btnSach_ChiTietBanSao.isClicked(clickPos)) {
                            // 1. Hỏi người dùng nhập ISBN
                            string isbn = FormNhapChuoi(font, "NHẬP ISBN ĐỂ XEM CHI TIẾT:", "ISBN:");
                            
                            if (!isbn.empty()) {
                                // 2. Tìm đầu sách trong mảng
                                DauSach* pTim = NULL;
                                for (int i = 0; i < dsdausach->n; i++) {
                                    if (dsdausach->nodes[i]->ISBN == isbn) {
                                        pTim = dsdausach->nodes[i];
                                        break;
                                    }
                                }

                                // 3. Nếu tìm thấy thì hiện Popup, không thì báo lỗi (hoặc lờ đi)
                                if (pTim != NULL) {
                                    FormXemChiTietBanSao(pTim, font);
                                } else {
                                    // Có thể dùng Form thông báo lỗi nếu muốn, hoặc cout tạm
                                    cout << "Không tìm thấy sách có ISBN: " << isbn << endl;
                                }
                            }
                        }
                    }
                    else if (currentTab == TAB_MUONTRA) {
                        if (btnMuon_in.isClicked(clickPos)) {
                            int maCanXem = FormNhapMaXoa(font, dsdocgia, L"NHẬP MÃ THẺ CẦN XEM:");
                            if (maCanXem != -1) {
                                    currentViewMaThe = maCanXem; // Lưu lại để hiển thị
                                    RefreshList(); 
                            }
                        }
                        if (btnMuon_Tra.isClicked(clickPos)) {
                            int maThe = -1; string maSach = "", tenSach = "";
                            if (FormTraSach(font, maThe, maSach, tenSach)) {
                                trasach(dsdocgia, dsdausach, maThe, maSach, tenSach);
                                if (currentViewMaThe == maThe) RefreshList(); // Cập nhật ngay nếu đang xem
                            }
                        }
                        if (btnMuon_Muon.isClicked(clickPos)) {
                            String thongbao;
                           string tenSach = "";
                            int maThe = -1;
                         if(FormMuonSach(font,maThe,tenSach)){
                          bool ketQua= muonsach(dsdausach,dsdocgia,tenSach,maThe,thongbao);
                          ShowMessage(font, thongbao);
                          if (ketQua== true) {
                    if (maThe != -1) {
                        currentViewMaThe = maThe;
                        RefreshList(); 
                    }
                }
    }
    
}
                    }
                }
            }
        }

        // --- 6. VẼ GIAO DIỆN ---
        Window.clear(Color(30, 32, 40)); // MÀU NỀN DARK BLUE GREY (Dễ chịu)

        // Vẽ Tab
        btnTabSach.draw(Window); btnTabDocGia.draw(Window); btnTabMuon.draw(Window);

        // Vẽ Bảng & Phân trang
        tableDisplay.draw(Window);
        btnPrevPage.draw(Window); btnNextPage.draw(Window);

        // Vẽ Nút Chức Năng (Tùy Tab)
        if (currentTab == TAB_DOCGIA) {
            btnAdd.draw(Window); btnDel.draw(Window); btnEdit.draw(Window); btnlock.draw(Window);
        }
        else if (currentTab == TAB_DAUSACH) {
            btnSach_Them.draw(Window); btnSach_Xoa.draw(Window); 
            btnSach_DieuChinh.draw(Window); btnSach_ThemBanSao.draw(Window);
            btnSach_ChiTietBanSao.draw(Window);
        }
        else if (currentTab == TAB_MUONTRA) {
            btnMuon_in.draw(Window); btnMuon_Tra.draw(Window);btnMuon_Muon.draw(Window);
        }

        Window.display();
    }
    return 0;
}
