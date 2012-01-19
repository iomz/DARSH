#!/bin/sh
cat servcert.pem servkey.pem servCAcert.pem rootcert.pem > serv.pem
cat clntcert.pem clntkey.pem rootcert.pem > clnt.pem
