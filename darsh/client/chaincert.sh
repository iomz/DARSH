#!/bin/sh
cat clntcert.pem clntkey.pem rootcert.pem > clnt.pem
cat servcert.pem servkey.pem servCAcert.pem rootcert.pem > serv.pem
