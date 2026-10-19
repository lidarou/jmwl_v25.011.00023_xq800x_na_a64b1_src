#/****************************************************************************/
#/*                                                                          */
#/*      Copyright 2021 by AIC Corporation. All rights reserved.             */
#/*                                                                          */
#/****************************************************************************/

# 导入cryptography库的相关模块和函数
from cryptography.hazmat.backends import default_backend
from cryptography.hazmat.primitives import serialization

from cryptography.hazmat.primitives import hashes
from cryptography.hazmat.primitives.asymmetric import padding

# 签名函数
def sign(data, private_key_file_name):
    # 从PEM文件中读取私钥数据
    key_file = open(private_key_file_name, 'rb')
    key_data = key_file.read()
    key_file.close()

    # 从PEM文件数据中加载私钥
    private_key = serialization.load_pem_private_key(
        key_data,
        password=None,
        backend=default_backend()
    )

    # 使用私钥对数据进行签名
    # 指定填充方式为PKCS1v15
    # 指定hash方式为sha256
    signature = private_key.sign(
        data,
        padding.PKCS1v15(),
        hashes.SHA256()
    )

    # 将签名数据写入结果文件中
    #signature_file = open(signature_file_name, 'wb')
    #signature_file.write(signature)
    #signature_file.close()

    # 返回签名数据
    return signature


def test_sign(data, private_key_file_name):
    # 从PEM文件中读取私钥数据
    key_file = open(private_key_file_name, 'rb')
    key_data = key_file.read()
    key_file.close()

    # 从PEM文件数据中加载私钥
    private_key = serialization.load_pem_private_key(
        key_data,
        password=None,
        backend=default_backend()
    )

    # 使用私钥对数据进行签名
    # 指定填充方式为PKCS1v15
    # 指定hash方式为sha256
    signature = private_key.sign(
        data,
        padding.PKCS1v15(),
        hashes.SHA256()
    )

    # 将签名数据写入结果文件中
    #signature_file = open(signature_file_name, 'wb')
    #signature_file.write(signature)
    #signature_file.close()

    # 返回签名数据
    return signature

# 验证函数
def test_verify(data, public_key_file_name, signature):
    # 从PEM文件中读取公钥数据
    key_file = open(public_key_file_name, 'rb')
    key_data = key_file.read()
    key_file.close()

    # 从PEM文件数据中加载公钥
    public_key = serialization.load_pem_public_key(
        key_data,
        backend=default_backend()
    )

    # 验证结果，默认为False
    verify_ok = False

    try:
        # 使用公钥对签名数据进行验证
        # 指定填充方式为PKCS1v15
        # 指定hash方式为sha256
        public_key.verify(
            signature,
            data,
            padding.PKCS1v15(),
            hashes.SHA256()
        )
    # 签名验证失败会触发名为InvalidSignature的exception
    except InvalidSignature:
        # 打印失败消息
        print('invalid signature!')
    else:
        # 验证通过，设置True
        verify_ok = True

    # 返回验证结果
    return verify_ok

def main():
    data = "liulanqing"
    real_data =  data.encode(encoding="utf-8")
    priv = "D:/workspace/test/sign_script/out/bl_priv_key.pem"
    pub = "D:/workspace/test/sign_script/out/bl_pub_key.pem"
#    sign(data, priv)
    signature = test_sign(real_data, priv)
    ret = test_verify(real_data, pub, signature)
    print(ret)


if __name__=='__main__':
    main()