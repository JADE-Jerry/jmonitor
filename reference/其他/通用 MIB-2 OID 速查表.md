# 通用 MIB-2 OID 速查表

说明：本表格基于 RFC 1213 标准定义的 MIB-2（1.3.6.1.2.1）核心对象，所有支持 SNMP 的网络设备通用。标注 `.0` 为标量对象可直接取值，无 `.0` 为表项需结合索引（如 ifIndex）查询。

## 一、系统信息组（system，1.3.6.1.2.1.1）

|对象名|完整 OID|数据类型|访问权限|用途说明|
|---|---|---|---|---|
|sysDescr|1.3.6.1.2.1.1.1.0|DisplayString|只读|设备完整描述，含硬件型号、操作系统版本、网络软件信息等|
|sysObjectID|1.3.6.1.2.1.1.2.0|OID|只读|厂商分配的设备类型唯一标识符|
|sysUpTime|1.3.6.1.2.1.1.3.0|TimeTicks|只读|设备网络管理模块上次重启至今的时间（单位：10ms）|
|sysContact|1.3.6.1.2.1.1.4.0|DisplayString|读写|设备管理员联系方式（姓名、电话、邮箱等）|
|sysName|1.3.6.1.2.1.1.5.0|DisplayString|读写|设备名称（通常为网络主机名）|
|sysLocation|1.3.6.1.2.1.1.6.0|DisplayString|读写|设备物理部署位置（如“机房A-机柜3”）|
|sysServices|1.3.6.1.2.1.1.7.0|Integer|只读|设备提供的OSI服务层级（1-7，叠加计算，如7代表全层级服务）|
## 二、接口组（interfaces，1.3.6.1.2.1.2）

|对象名|完整 OID|数据类型|访问权限|用途说明|
|---|---|---|---|---|
|ifNumber|1.3.6.1.2.1.2.1.0|Integer|只读|设备拥有的网络接口总数|
|ifIndex|1.3.6.1.2.1.2.2.1.1|Integer|只读|接口唯一索引（用于关联其他接口表项）|
|ifDescr|1.3.6.1.2.1.2.2.1.2|DisplayString|只读|接口描述（如“Gi0/0/1”“LoopBack0”）|
|ifType|1.3.6.1.2.1.2.2.1.3|Integer|只读|接口类型（6=以太网，22=串行接口，1=其他）|
|ifMtu|1.3.6.1.2.1.2.2.1.4|Integer|只读|接口最大传输单元（MTU）值|
|ifSpeed|1.3.6.1.2.1.2.2.1.5|Gauge32|只读|接口速率（单位：bps，如100000000=100Mbps）|
|ifPhysAddress|1.3.6.1.2.1.2.2.1.6|PhysAddress|只读|接口数据链路层地址（如以太网MAC地址）|
|ifAdminStatus|1.3.6.1.2.1.2.2.1.7|Integer|读写|接口管理状态（1=启用，2=禁用，3=测试）|
|ifOperStatus|1.3.6.1.2.1.2.2.1.8|Integer|只读|接口实际运行状态（1=up，2=down，3=测试）|
|ifInOctets|1.3.6.1.2.1.2.2.1.10|Counter32|只读|接口入方向累计字节数（含所有协议帧）|
|ifInUcastPkts|1.3.6.1.2.1.2.2.1.11|Counter32|只读|接口入方向累计单播数据包数|
|ifOutOctets|1.3.6.1.2.1.2.2.1.16|Counter32|只读|接口出方向累计字节数（含所有协议帧）|
|ifOutUcastPkts|1.3.6.1.2.1.2.2.1.17|Counter32|只读|接口出方向累计单播数据包数|
## 三、IP组（ip，1.3.6.1.2.1.4）

|对象名|完整 OID|数据类型|访问权限|用途说明|
|---|---|---|---|---|
|ipForwarding|1.3.6.1.2.1.4.1.0|Integer|读写|是否启用IP转发（1=启用，2=禁用；路由器默认启用，主机默认禁用）|
|ipDefaultTTL|1.3.6.1.2.1.4.2.0|Integer|读写|本地生成IP数据包的默认TTL值（通常为64）|
|ipInReceives|1.3.6.1.2.1.4.3.0|Counter32|只读|接收的IP数据包总数（含错误包）|
|ipInDelivers|1.3.6.1.2.1.4.9.0|Counter32|只读|成功投递到上层协议（TCP/UDP/ICMP等）的IP数据包数|
|ipOutRequests|1.3.6.1.2.1.4.10.0|Counter32|只读|本地生成并发送的IP数据包总数（不含转发包）|
## 四、ICMP组（icmp，1.3.6.1.2.1.5）

|对象名|完整 OID|数据类型|访问权限|用途说明|
|---|---|---|---|---|
|icmpInMsgs|1.3.6.1.2.1.5.1.0|Counter32|只读|接收的ICMP消息总数|
|icmpInEchos|1.3.6.1.2.1.5.2.0|Counter32|只读|接收的ICMP Echo请求数（即Ping请求数）|
|icmpOutEchosRep|1.3.6.1.2.1.5.10.0|Counter32|只读|发送的ICMP Echo应答数（即Ping应答数）|
|icmpOutErrors|1.3.6.1.2.1.5.14.0|Counter32|只读|发送ICMP消息时产生的错误总数|
## 五、TCP组（tcp，1.3.6.1.2.1.6）

|对象名|完整 OID|数据类型|访问权限|用途说明|
|---|---|---|---|---|
|tcpRtoAlgorithm|1.3.6.1.2.1.6.1.0|Integer|只读|TCP重传超时（RTO）计算算法（1=标准，2=常数，3=指数退避）|
|tcpRtoMin|1.3.6.1.2.1.6.2.0|Integer|只读|最小TCP重传超时时间（单位：ms，通常为100ms）|
|tcpActiveOpens|1.3.6.1.2.1.6.4.0|Counter32|只读|主动发起的TCP连接数（如客户端访问服务器）|
|tcpPassiveOpens|1.3.6.1.2.1.6.5.0|Counter32|只读|被动接受的TCP连接数（如服务器监听端口接收连接）|
|tcpCurrEstab|1.3.6.1.2.1.6.9.0|Gauge32|只读|当前已建立的TCP连接数（ESTABLISHED状态）|
|tcpInSegs|1.3.6.1.2.1.6.10.0|Counter32|只读|接收的TCP段总数（含错误段）|
|tcpOutSegs|1.3.6.1.2.1.6.11.0|Counter32|只读|发送的TCP段总数（不含重传段）|
## 六、UDP组（udp，1.3.6.1.2.1.7）

|对象名|完整 OID|数据类型|访问权限|用途说明|
|---|---|---|---|---|
|udpInDatagrams|1.3.6.1.2.1.7.1.0|Counter32|只读|接收的UDP数据报总数（含错误数据报）|
|udpNoPorts|1.3.6.1.2.1.7.2.0|Counter32|只读|目标端口不可达的UDP数据报数（触发ICMP端口不可达响应）|
|udpOutDatagrams|1.3.6.1.2.1.7.4.0|Counter32|只读|发送的UDP数据报总数|
|udpLocalPort|1.3.6.1.2.1.7.5.1.2|Integer|只读|UDP监听端口表项（需结合索引查询具体监听端口）|
## 七、SNMP组（snmp，1.3.6.1.2.1.11）

|对象名|完整 OID|数据类型|访问权限|用途说明|
|---|---|---|---|---|
|snmpInPkts|1.3.6.1.2.1.11.1.0|Counter32|只读|接收的SNMP消息总数|
|snmpOutPkts|1.3.6.1.2.1.11.2.0|Counter32|只读|发送的SNMP消息总数|
|snmpInBadVersions|1.3.6.1.2.1.11.3.0|Counter32|只读|SNMP版本不兼容的消息数（如设备只支持v2c，接收v3消息）|
|snmpInBadCommunityNames|1.3.6.1.2.1.11.4.0|Counter32|只读|团体名错误的SNMP消息数（认证失败）|

      关键备注：1. Counter32类型数据会随时间累计溢出，建议使用SNMPv2c的Counter64类型或定期重置统计；2. 表项OID需拼接索引值（如ifInOctets.1对应索引为1的接口入字节数）；3. 所有OID跨厂商通用，私有功能需查询厂商私有分支（1.3.6.1.4.1）。
    
> （注：文档部分内容可能由 AI 生成）