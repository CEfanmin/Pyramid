# -*- encoding: utf-8 -*-
'''
@File    :   kafka_test.py
@Time    :   2021/03/04 11:10:13
@Author  :   fanmin
'''
import json
from pprint import pprint
from kafka import KafkaProducer, KafkaConsumer


class KafkaCourier(object):
    def __init__(self, server_addr ,producer_topic, consumer_topic):
        """
        初始化kafka地址和topic
        """
        self._producer_topic = producer_topic
        self._consumer_topic = consumer_topic
        self._server_addr = server_addr
        self.producer = KafkaProducer(bootstrap_servers=server_addr,
                                        value_serializer=lambda m: json.dumps(m).encode('ascii'))
        self.consumer =  KafkaConsumer(consumer_topic, bootstrap_servers=server_addr)

    def numeric_send(self, configer=None):
        """
        每一条消息的数值转换
        """
        for msg in self.consumer:
            recv = "%s:%d:%d: key=%s value=%s" % (msg.topic, msg.partition, msg.offset, msg.key, msg.value)
            msg_body = msg.value.decode(encoding="utf-8", errors="ignore")
            # msg_body = json.loads(msg_body)
            msg_body = eval(msg_body)
            # pprint(msg_body)
            # TODO: 针对字典中的每个字段进行相应的处理，并发送
            src_ip = msg_body["source_endpoint_ip"]
            print(src_ip)
            # 生产消息
            # try:
            #     self.producer.send(self._producer_topic, msg_body)
            #     self.producer.flush()
            # except Exception as ex:
            #     print('Exception in publishing message')
            #     print(str(ex))


if __name__ == "__main__":
    """
    测试kafka
    """
    Courier = KafkaCourier(server_addr = ['localhost:9092'],
                            producer_topic=None,
                            consumer_topic='events')
    Courier.numeric_send(configer=None)
