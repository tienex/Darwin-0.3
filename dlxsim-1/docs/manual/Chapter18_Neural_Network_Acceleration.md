# Chapter 18: Neural Network Acceleration (N Extension)

## 18.1 Neural Network Forward Pass

```assembly
# Dense (Fully Connected) Layer
nn.dense.fwd    mr_out, mr_in, mr_weights, mr_bias
                # out ← in × weights + bias

# Activation Functions
nn.relu         mr_out, mr_in       # ReLU: out ← max(0, in)
nn.gelu         mr_out, mr_in       # GELU activation
nn.sigmoid      mr_out, mr_in       # Sigmoid: out ← 1/(1+e^(-in))
nn.tanh         mr_out, mr_in       # Tanh activation
nn.swish        mr_out, mr_in       # Swish: out ← in × sigmoid(in)
nn.softmax      mr_out, mr_in, axis # Softmax activation

# Normalization
nn.batchnorm.fwd mr_out, mr_in, mr_gamma, mr_beta, mr_mean, mr_var, epsilon
                # Batch normalization forward

nn.layernorm.fwd mr_out, mr_in, mr_gamma, mr_beta, epsilon
                # Layer normalization forward

# Convolution
nn.conv2d.fwd   mr_out, mr_in, mr_kernel, mr_bias, stride, padding
                # 2D convolution

# Pooling
nn.pool.max     mr_out, mr_in, kernel_size, stride, padding
nn.pool.avg     mr_out, mr_in, kernel_size, stride, padding
nn.pool.global.max  mr_out, mr_in    # Global max pooling
nn.pool.global.avg  mr_out, mr_in    # Global average pooling
```

## 18.2 Neural Network Backward Pass (Training)

```assembly
# Dense Layer Backward
nn.dense.bwd    mr_grad_in, mr_grad_weights, mr_grad_bias, \
                mr_grad_out, mr_input, mr_weights
                # Compute gradients for backpropagation

# Activation Gradients
nn.relu.grad    mr_grad_in, mr_grad_out, mr_input
nn.sigmoid.grad mr_grad_in, mr_grad_out, mr_output
nn.tanh.grad    mr_grad_in, mr_grad_out, mr_output
nn.gelu.grad    mr_grad_in, mr_grad_out, mr_input

# Convolution Backward
nn.conv2d.bwd.input  mr_grad_in, mr_grad_out, mr_kernel, stride, padding
nn.conv2d.bwd.kernel mr_grad_kernel, mr_grad_out, mr_input, stride, padding
nn.conv2d.bwd.bias   mr_grad_bias, mr_grad_out

# Normalization Backward
nn.batchnorm.bwd mr_grad_in, mr_grad_gamma, mr_grad_beta, \
                 mr_grad_out, mr_input, mr_gamma, mr_mean, mr_var, epsilon
```

## 18.3 Loss Functions and Optimizers

```assembly
# Loss Functions with Gradients
nn.loss.crossent     vr_loss, mr_logits, mr_labels
nn.loss.crossent.grad mr_grad, mr_logits, mr_labels

nn.loss.mse          vr_loss, mr_pred, mr_target
nn.loss.mse.grad     mr_grad, mr_pred, mr_target

# Optimizers
nn.opt.sgd           mr_params, mr_grads, learning_rate
nn.opt.sgd.momentum  mr_params, mr_grads, mr_velocity, lr, momentum
nn.opt.adam          mr_params, mr_grads, mr_m, mr_v, lr, beta1, beta2, epsilon, t
nn.opt.rmsprop       mr_params, mr_grads, mr_v, lr, alpha, epsilon
```
