#ifndef TS_TRANSMIT_INTERNAL_NOTE_PCAP_FILE_INPUT_H_
#define TS_TRANSMIT_INTERNAL_NOTE_PCAP_FILE_INPUT_H_

#include "lccl/bit.h"
#include "lccl/file.h"
#include "lccl/socket.h"
#include "node/base_node.h"

TSTM_BEGIN_NAMESPACE

class PcapFileInputNode : public BaseNode
{
public:
    static constexpr int kLoopForever = -1;

    struct Param
    {
        std::string file_name;
        std::string hint_ip;
        uint16_t hint_port;
        int loop = kLoopForever;
        bool real_time = true;
        int64_t error_time_ns = 60 * kSecInNs;
    };

    struct BasicInfo
    {
        int64_t duration_ms = 0;
    };

public:
    explicit PcapFileInputNode(const std::string &id);
    virtual ~PcapFileInputNode();

    virtual bool Init(const rapidjson::Value &param_val);
    virtual void Deinit();

    virtual std::shared_ptr<TokenList> Do(std::shared_ptr<TokenList> in_token_list);

    const BasicInfo *GetBasicInfo() const;

private:
    enum NetworkTypes : int64_t
    {
        kEthernet = 1,
        kCooked = 113,
    };

    enum class PacketTypes
    {
        kUdp = 0,
        kTruncation,
        kUnsupported,
    };

    struct Packet
    {
        PacketTypes type = PacketTypes::kUnsupported;
        int64_t timestamp = 0;
        std::string src_ip;
        std::string dst_ip;
        uint16_t src_port = 0;
        uint16_t dst_port = 0;

        int64_t out_timestamp = 0;
        const uint8_t *data = nullptr;
        size_t size = 0;
    };

private:
    bool ParseParam(const rapidjson::Value &param_val);
    bool ParseBasicInfo();

    void ReadPacket();
    bool ReadEthernetHeader(lccl::bit::Reader &reader, size_t size);
    bool IsHint();

private:
    std::shared_ptr<Param> param_;
    bool hint_init_;

    std::shared_ptr<lccl::file::IFileMmap> file_mmap_;
    std::shared_ptr<BasicInfo> basic_info_;

    const uint8_t *pcap_begin_;
    const uint8_t *pcap_end_;
    size_t pcap_size_;

    lccl::bit::Reader pcap_reader_;
    lccl::bit::SavePoint packet_start_sp_;
    std::vector<char> ip_buffer_;

    lccl::bit::ByteOrders bo_;
    int64_t version_;
    int64_t time_precision_;
    NetworkTypes network_type_;

    size_t loop_;
    bool loop_end_;

    std::shared_ptr<Packet> curr_packet_;
    int64_t prev_timestamp_;
    int64_t prev_out_timestamp_;

    std::shared_ptr<TokenList> out_token_list_;
};

TSTM_END_NAMESPACE

#endif // !TS_TRANSMIT_INTERNAL_NOTE_PCAP_FILE_INPUT_H_
