/**
 * @file UnrolledLinkedList.h
 * @author Conless Pan (conlesspan@outlook.com)
 * @brief
 * @version 0.1
 * @date 2022-12-09
 *
 * @copyright Copyright (c) 2022
 *
 */

#ifndef BOOKSTORE_LIST_ULL_H
#define BOOKSTORE_LIST_ULL_H

#include <bits/stdc++.h>

enum StorageType {
    RAMStorage,
    ROMStorage
};

StorageType cur;

double rd_time, wt_time;

namespace bookstore {

namespace list {

/**
 * @brief Class UnrolledLinkedList
 * @details The main type of file storage system
 */
class UnrolledLinkedList {
  private:
    // The type of key
    typedef char KeyType[72];

    // The type of data
    typedef int DataType;

    // The type of index
    typedef unsigned int IndexType;

    // The size of those types
    const size_t kSizeofKey = sizeof(KeyType);
    const size_t kSizeofData = sizeof(DataType);
    const size_t kSizeofIndex = sizeof(IndexType);
    const size_t kSizeofNode = kSizeofKey + kSizeofData + kSizeofIndex * 2;

  private:
    /**
     * @brief Structure type Node
     * @details Used to record all the data in a node of ull. Similar to struct node in traditional linkedlist.
     */
    struct Node {
        // The key of this node
        std::string key;
        // The pos of this node, i.e. the place it is located on in file_name.bin
        IndexType pos;
        // The data of this node
        DataType data;
        // The next "pointer" of this node, i.e. the pos of its next node.
        IndexType next;

        // Constructor of Node
        Node() = default;
        // Initializer of Node
        Node(std::string key, IndexType pos, DataType data, IndexType next = 0) // Initializer for Node
            : key(key), pos(pos), data(data), next(next) {}
    };

  private:
    // The maximum size of a single list
    const DataType max_block_size;

    // The head, tail "pointer" of each block
    std::vector<IndexType> head, tail;

    // The size of each block
    std::vector<IndexType> siz;

    // The file name
    std::string file_name;

    // The total number of data, not included the deleted ones
    DataType cnt;

    // The file I/O variable
    std::fstream file;

  public:
    // Constructor of ull
    UnrolledLinkedList(const std::string file_path, const bool inherit_tag = false, const IndexType block_size = 1000);

    // Destructor of ull
    ~UnrolledLinkedList();

  public:
    std::vector<Node> ram;
    // Read a node in file_name.bin
    Node ReadNode(IndexType pos);

    // Write or rewrite a node in file_name.bin
    void WriteNode(IndexType pos, Node now);

    // Insert a node in the selected block
    IndexType InsertData(IndexType pos, const std::string key, IndexType num, DataType data);

    // Simplify the ull
    void Simplify();

    // Output the current data of ull
    void Output();

  public:
    // Insert a pair of key and data
    void insert(const std::string key, DataType data);

    // Find a vector of data by the given key
    std::vector<DataType> find(const std::string key);

    // Erase a pair of key and data
    void erase(const std::string key, DataType data);
};

} // namespace list
} // namespace bookstore

#endif
/**
 * @file UnrolledLinkedList.cc
 * @author Conless Pan (conlesspan@outlook.com)
 * @brief The implementation for UnrolledLinkedList.h
 * @version 0.1
 * @date 2022-12-09
 *
 * @copyright Copyright (c) 2022
 *
 */

#include <bits/stdc++.h>

namespace bookstore {

namespace list {

/**
 * @brief Construct a new Unrolled Linked List:: Unrolled Linked List object
 * @details Create a new ull, or inherit the data from the last test point
 * @param file_path
 * @param inherit_tag
 * @param block_size
 */
UnrolledLinkedList<kMaxKeyLen>::UnrolledLinkedList(const std::string file_path, const bool inherit_tag, const IndexType block_size)
    : file_name(file_path), max_block_size(block_size) {

    // Initialize and clear
    cnt = 0;
    head.clear();
    tail.clear();
    siz.clear();

    if (cur == RAMStorage) {
        ram.resize(100005);
        std::ifstream input_tmp(file_name + ".dat");
        if (input_tmp.good() && inherit_tag) {
            input_tmp >> cnt;
            ram.resize(cnt);
            for (int i = 0; i < cnt; i++)
                input_tmp >> ram[i].key >> ram[i].pos >> ram[i].data >> ram[i].next;
            IndexType len_tmp;
            input_tmp >> len_tmp;
            head.resize(len_tmp);
            tail.resize(len_tmp);
            siz.resize(len_tmp);
            for (IndexType i = 0; i < len_tmp; i++)
                input_tmp >> head[i] >> tail[i] >> siz[i];
        }
        return;
    }

    // The file_name.bin is used to storage all key and data, written in binary text. Here use it to determine whether to
    // inherit data from former test points
    std::ifstream input(file_name + ".bin");

    if (inherit_tag && input.good()) { // If the program should inherit data
        // The file_name.dat is used to storage key data which was saved in RAM, i.e. cnt, head, tail and siz. So here the
        // program first read the key data of the last ull
        input.close();
        input.open(file_name + ".dat");

        // Inputing...
        IndexType len; // The number of blocks, similarly hereafter
        input >> cnt >> len;
        while (len--) {
            IndexType head_num, tail_num, siz_num;
            input >> head_num >> tail_num >> siz_num;
            head.push_back(head_num);
            tail.push_back(tail_num);
            siz.push_back(siz_num);
        }
    } else { // Create a new file_name.bin and reset it as a clear file
        std::ofstream create(file_name + ".bin", std::ios::out | std::ios::trunc);
        create.close();
    }

    // Open the binary data file
    file.open(file_name + ".bin", std::ios::in | std::ios::out);

    return;
}

/**
 * @brief Destroy the Unrolled Linked List:: Unrolled Linked List object
 * @details Destory a ull, and output its key data for reuse
 */
UnrolledLinkedList<kMaxKeyLen>::~UnrolledLinkedList() {
    if (cur == RAMStorage) {
        std::ofstream output_tmp(file_name + ".dat", std::ios::out | std::ios::trunc);
        IndexType len_tmp = head.size();
        output_tmp << cnt << '\n';
        for (int i = 0; i < cnt; i++)
            output_tmp << ram[i].key << " " << ram[i].pos << " " << ram[i].data << " " << ram[i].next << '\n';
        output_tmp << len_tmp << '\n';
        for (IndexType i = 0; i < len_tmp; i++)
            output_tmp << head[i] << ' ' << tail[i] << ' ' << siz[i] << '\n';
        return;
    }

    // The file_name.dat is used to storage key data which was saved in RAM, i.e. cnt, head, tail and siz. Here the program
    // record them in normal text mode
    std::ofstream output(file_name + ".dat", std::ios::out | std::ios::trunc);
    // Outputing...
    IndexType len = head.size();
    output << cnt << " " << len << '\n';
    for (IndexType i = 0; i < len; i++)
        output << head[i] << ' ' << tail[i] << ' ' << siz[i] << '\n';
    output.close();
    return;
}

/**
 * @brief Read a node in file_name.bin
 * @details Read the key of KeyType(std::string), pos of IndexType(uint), data of DataType(int) and next of IndexType(uint) in
 * the binary file
 * @param pos
 * @return Node
 */
UnrolledLinkedList<kMaxKeyLen>::Node UnrolledLinkedList<kMaxKeyLen>::ReadNode(IndexType pos) {
    if (cur == RAMStorage)
        return ram[pos - 1];
    double fir = (double)clock() / CLOCKS_PER_SEC;
    Node ret;
    ret.pos = pos;
    // Move the read head, (pos - 1) suggests that the selected data is located at the (pos - 1)th node of binary file
    file.seekg((pos - 1) * sizeof(ret));

    // Inputing...
    KeyType str;
    int a = file.tellg();
    file.read(reinterpret_cast<char *>(&ret), sizeof(ret));
    double sec = (double)clock() / CLOCKS_PER_SEC;
    rd_time += sec - fir;
    return ret;
}

/**
 * @brief Write or rewrite a node in file_name.bin
 * @details Write the key of KeyType(std::string), pos of IndexType(uint), data of DataType(int) and next of IndexType(uint) in
 * the binary file
 * @param pos
 * @param now
 */
void UnrolledLinkedList<kMaxKeyLen>::WriteNode(IndexType pos, Node now) {
    if (cur == RAMStorage) {
        ram[pos - 1] = now;
        return;
    }
    double fir = (double)clock() / CLOCKS_PER_SEC;
    // Move the write head, (pos - 1) suggests that the selected data is located at the (pos - 1)th node of binary file
    file.seekp((pos - 1) * sizeof(now));

    // Outputing...
    file.write(reinterpret_cast<char *>(&now), sizeof(now));
    file.seekg((pos - 1) * sizeof(now));
    int a = file.tellg();
    file.read(reinterpret_cast<char *>(&now), sizeof(now));
    double sec = (double)clock() / CLOCKS_PER_SEC;
    wt_time += sec - fir;
    return;
}

/**
 * @brief Insert a node in the selected block
 * @details Insert a new node in the selected block in order
 * @param pos
 * @param key
 * @param num
 * @param data
 * @return UnrolledLinkedList<kMaxKeyLen>::IndexType
 */
UnrolledLinkedList<kMaxKeyLen>::IndexType UnrolledLinkedList<kMaxKeyLen>::InsertData(IndexType pos, const std::string key, IndexType num,
                                                             DataType data) {
    // Increase the size of the current block
    siz[pos]++;

    // Read the data of the head node of the current block
    Node now = ReadNode(head[pos]);
    if (key < now.key || (key == now.key && data < now.data)) {
        head[pos] = num;
        return now.pos;
    }

    // Search the node in the block one by one
    while (true) {
        // Get the pos of the next
        IndexType nex_pos = now.next;

        if (now.key == key && now.data == data) {
            siz[pos]--;
            return 0;
        }

        if (!nex_pos) { // If nextis null, i.e. we've arrived to the tail, then just insert the node here
            tail[pos] = num;
            now.next = num;
            WriteNode(now.pos, now);
            return 0;
        }

        Node nex = ReadNode(nex_pos); // Or determine whether the node can be inserted between
        if (key < nex.key || (key == nex.key && data < nex.data)) {
            now.next = num;
            WriteNode(now.pos, now);
            return nex_pos;
        }

        // Move to the next node
        now = nex;
    }
}

/**
 * @brief Simplify the ull
 * @details Simplify a ull in two following ways. First, find if there're some large blocks and divide them. Then, find if
 * there're some consecutive small blocks and merge them.
 */
void UnrolledLinkedList<kMaxKeyLen>::Simplify() {
    IndexType len = head.size();
    for (int i = 0; i < len; i++) {
        if (!siz[i]) {
            head.erase(head.begin() + i);
            tail.erase(tail.begin() + i);
            siz.erase(siz.begin() + i);
        }
    }
    len = head.size();
    for (IndexType i = 0; i < len; i++) {
        if (siz[i] >= max_block_size * 3) { // If we find a large block, divide it from the mid
            // Start from the head node
            Node now = ReadNode(head[i]), las;
            for (IndexType j = 1; j < max_block_size; j++)
                now = ReadNode(now.next);

            // Here now := the last node of the first block, with max_block_size
            tail.insert(tail.begin() + i + 1, tail[i]);
            tail[i] = now.pos;
            IndexType tmp = now.next;
            now.next = 0;

            // Rewrite the tail node
            WriteNode(now.pos, now);

            // Get the data of the head node of the new block
            now = ReadNode(tmp);
            head.insert(head.begin() + i + 1, now.pos);
            siz.insert(siz.begin() + i + 1, siz[i] - max_block_size);
            siz[i] -= max_block_size;
        } else if (i < len - 1) {
            if (siz[i] + siz[i + 1] <= max_block_size) { // If we find a small block, which, can be merged with the next block
                // Read the data of the tail
                Node now = ReadNode(tail[i]);

                // Connext it with the head of the next block
                now.next = head[i + 1];
                WriteNode(now.pos, now);

                // Edit the data
                siz[i] += siz[i + 1];
                tail[i] = tail[i + 1];
                siz.erase(siz.begin() + i + 1);
                head.erase(head.begin() + i + 1);
                tail.erase(tail.begin() + i + 1);
            }
        }
    }
}

/**
 * @brief Output the current data of ull
 *
 */
void UnrolledLinkedList<kMaxKeyLen>::Output() {
    IndexType len = head.size();
    std::cout << cnt << '\n';
    for (IndexType i = 0; i < len; i++) {
        std::cout << "Block " << i << ' ' << siz[i] << '\n';
        IndexType pos = head[i];
        while (pos) {
            Node now = ReadNode(pos);
            std::cout << now.key << ' ' << now.pos << ' ' << now.data << ' ' << now.next << '\n';
            pos = now.next;
        }
        std::cout << '\n';
    }
}

/**
 * @brief Insert a pair of key and data
 * @details Insert a pair in order into the ull and maintain the size of ull
 * @param key
 * @param data
 */
void UnrolledLinkedList<kMaxKeyLen>::insert(const std::string key, DataType data) {
    // Increase the count of current node
    cnt++;
    if (cnt == 1) { // If it is the first node
        head.push_back(cnt);
        tail.push_back(cnt);
        siz.push_back(1);
        WriteNode(cnt, Node(key, cnt, data));
    } else { // Find a correct place to insert
        IndexType len = head.size();

        for (IndexType i = 0; i < len; i++) {
            // If it is the last block, we have to insert it
            if (i == len - 1) {
                IndexType ret_pos = InsertData(i, key, cnt, data);
                WriteNode(cnt, Node(key, cnt, data, ret_pos));
                break;
            }

            // Or else, insert it in the first block such that the head of the next block is greater than it
            Node nex = ReadNode(head[i + 1]);
            if (key < nex.key || (key == nex.key && data < nex.data)) {
                IndexType ret_pos = InsertData(i, key, cnt, data);
                WriteNode(cnt, Node(key, cnt, data, ret_pos));
                break;
            }
        }
    }
    Simplify();
    // Output();
}

/**
 * @brief Find a vector of data by the given key
 * @details Find all of the data corresponding the given key, in the sort of the second keyword
 * @param key
 * @return std::vector<UnrolledLinkedList<kMaxKeyLen>::DataType>
 */
std::vector<UnrolledLinkedList<kMaxKeyLen>::DataType> UnrolledLinkedList<kMaxKeyLen>::find(const std::string key) {
    std::vector<DataType> ret;
    ret.clear();

    // If empty, directly return the empty vector
    if (!cnt)
        return ret;

    IndexType len = head.size();
    for (IndexType i = 0; i < len; i++) {
        // Search in the i-th block (0-base)
        Node now = ReadNode(head[i]);

        // If the min data is even greater than the key
        if (now.key > key)
            return ret;

        // Determine whether the data can be found in the current block
        Node tai = ReadNode(tail[i]);
        if (tai.key >= key) { // If possible
            while (true) {
                if (now.key == key) // Found
                    ret.push_back(now.data);
                else if (now.key > key) // Impossible then
                    return ret;
                if (!now.next) // Tail
                    break;
                now = ReadNode(now.next);
            }
        }
    }
    return ret;
}

/**
 * @brief Erase a pair of key and data
 * @details Erase a pair of given key and data, throw error when not found. Maintaining the size of the block.
 * @param key
 * @param data
 */
void UnrolledLinkedList<kMaxKeyLen>::erase(const std::string key, DataType data) {
    // If empty, then the data cannot be found
    if (!cnt)
        return;

    IndexType len = head.size();
    for (IndexType i = 0; i < len; i++) {
        // Search in the i-th block (0-base)
        Node now = ReadNode(head[i]);
        if (now.key > key)
            return;
        Node tai = ReadNode(tail[i]);
        if (tai.key >= key) {
            Node las;
            while (true) {
                if (now.key == key && now.data == data) { // Found
                    // cnt--;
                    siz[i]--;
                    if (now.pos == head[i] && now.pos == tail[i]) { // The only node in the block
                        head.erase(head.begin() + i);
                        tail.erase(tail.begin() + i);
                        siz.erase(siz.begin() + i);
                    } else if (now.pos == head[i]) { // Head node of the block
                        head[i] = now.next;
                    } else if (now.pos == tail[i]) { // Tail node of the block
                        tail[i] = las.pos;
                        las.next = 0;
                        WriteNode(las.pos, las);
                    } else { // Otherwise
                        las.next = now.next;
                        WriteNode(las.pos, las);
                    }
                    now.data = -1; // A tag of being "erased", though not used
                    WriteNode(now.pos, now);
                    Simplify();
                    return;
                } else if (now.key > key) {
                    return;
                }
                if (!now.next)
                    break;
                las = now;
                now = ReadNode(now.next);
            }
        }
    }
    return;
}

} // namespace list

} // namespace bookstore

int main(int argc, char *argv[]) {
    if (argc >= 2 && !strcmp(argv[1], "--storage-type=ram"))
        cur = RAMStorage;
    else
        cur = ROMStorage;
    std::ios::sync_with_stdio(false);
    double time_used[4] = {0};
    int T;
    std::cin >> T;
    bookstore::list::UnrolledLinkedList l("test", true, 1000);
    while (T--) {
        double firr = (double)clock() / CLOCKS_PER_SEC;
        std::string opt;
        std::cin >> opt;
        if (opt == "insert") {
            double fir = (double)clock() / CLOCKS_PER_SEC;
            std::string s;
            int data;
            std::cin >> s >> data;
            l.insert(s, data);
            double sec = (double)clock() / CLOCKS_PER_SEC;
            time_used[0] += sec - fir;
        } else if (opt == "find") {
            double fir = (double)clock() / CLOCKS_PER_SEC;
            std::string s;
            std::cin >> s;
            std::vector<int> ret = l.find(s);
            if (!ret.size())
                std::cout << "null";
            else {
                for (auto i : ret)
                    std::cout << i << ' ';
            }
            std::cout << '\n';
            double sec = (double)clock() / CLOCKS_PER_SEC;
            time_used[1] += sec - fir;
        } else if (opt == "delete") {
            double fir = (double)clock() / CLOCKS_PER_SEC;
            std::string s;
            int data;
            std::cin >> s >> data;
            l.erase(s, data);
            double sec = (double)clock() / CLOCKS_PER_SEC;
            time_used[2] += sec - fir;
        }
        // std::cout << opt << '\n';
        // l.Output();
        double secc = (double)clock() / CLOCKS_PER_SEC;
        time_used[3] += secc - firr;
    }
    printf("%.6lf\n", (double)clock() / CLOCKS_PER_SEC);
    printf("%.6lf %.6lf %.6lf %.6lf\n%.6lf %.6lf", time_used[0], time_used[1], time_used[2], time_used[3], rd_time, wt_time);
    return 0;
}
