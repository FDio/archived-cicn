#!/bin/bash

mkdir -p ~/.vicn/ssh_client_cert/ && ssh-keygen -t rsa -N "" -f ~/.vicn/ssh_client_cert/ssh_client_key
