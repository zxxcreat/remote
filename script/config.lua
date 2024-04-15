require "script/common"
gateway_config={
    
    -- ipv6 和ip4 类型 对外
    ip_type=6;
    ip="127.0.0.1",
    port=58562,
    --默认转发的id 一个网关对应一个逻辑服务器？ 不能一对多
    --可以多对1 不能1对多
    defult_id=2,
    -- 群组id 八位整数
    group_id=1,
    ---密钥 lld s事件
    key_join="gasdag%lldhjhgfdsdd",
    check_str="g564sd15sd8d52dd",



    local_ip="127.0.0.1",
    local_port=15000,
}
center={
	--中心转发服务器 
    ip="127.0.0.1",
    port=15001,
    -- 群组id 八位整数
    group_id=0,
    -- 最长35
    password="df58h156h%lld9tht68ffg",
}
servers={
    --                             
    -- 服务器id 一个64位够了 
    server_id=1;
    login={
        group_id=2,
        ip="127.0.0.1",
        port=15002,

    },
    logic={
        group_id=3,
        ip="127.0.0.1",
        port=15003,
    },
}
-- gate 对应的逻辑服务器 ->login
